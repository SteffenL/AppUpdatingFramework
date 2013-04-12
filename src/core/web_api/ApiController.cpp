// SSL
// Not using SSL because OpenSSL added about 2 MB to the executable file size.
//
//#include <Poco/Net/HTTPSClientSession.h>
//
// Imports for SSL:
// Debug:   PocoNetSSLd.lib;libeay32.lib;ssleay32.lib;
// Release: PocoNetSSLmt.lib;libeay32mt.lib;ssleay32mt.lib;
//
// Include dirs:
// $(OPENSSLDIR)\inc32;$(POCODIR)\Crypto\include;$(POCODIR)\NetSSL_OpenSSL\include

#include "ApiController.h"
#include "exceptions.h"
#include "certificates/StartComCaCert.h"

#include <cyassl/ssl.h>
#include <curl/curl.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <json/json.h>

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <stdexcept>
#include <sstream>

namespace aufw { namespace web_api {

CURLcode sslContextCallback(void* ctx) {
    // Load CA certificate from memory
    certificates::StartComCaCert cert;
    const unsigned char* certData = cert.GetData();
    CyaSSL_CTX_load_verify_buffer(
        reinterpret_cast<CYASSL_CTX*>(ctx),
        certData,
        cert.GetSize(),
        SSL_FILETYPE_PEM);
    return CURLE_OK;
}

//
// Implementation
//

class ApiController::Impl {
public:
    Impl(const std::string& host, bool useSsl = true);
    void SendRequest(const std::string& uri, const Json::Value& jsonRequestData, Json::Value& jsonResponseData,
        const std::string& httpMethod);
    void ReceiveResponse(Json::Value& jsonResponseData);

private:
    static const int MAX_REQUEST_TRY_COUNT = 3;
    static const int MAX_RESPONSE_TRY_COUNT = 3;
    static const int MAX_RESPONSE_LENGTH_TO_RETRY =  2 * 1024 * 1024;
    //Poco::Net::HTTPClientSession m_session;
    std::string m_host;
    bool m_useSsl;
    long m_httpStatus;

    std::string getFullUri(const std::string& uri);
    void setHttpMethodOpt(curlpp::Easy& easy, const std::string& method);
};

ApiController::Impl::Impl(const std::string& host, bool useSsl) : m_host(host), m_useSsl(useSsl) /*: m_session(host)*/ {
    // SSL
    /*const Context::Ptr context = new Context(Context::CLIENT_USE, std::string(), std::string(), std::string());
    HTTPSClientSession session(context);*/
    //m_session.setTimeout(Poco::Timespan(5, 0));
}

void ApiController::Impl::SendRequest(const std::string& uri, const Json::Value& jsonRequestData, Json::Value& jsonResponseData,
    const std::string& httpMethod) {
    Json::FastWriter jsonWriter;
    std::stringstream postData;
    postData << jsonWriter.write(jsonRequestData);
    std::stringstream responseData;

    try {
        using namespace curlpp;
        Easy easy;
        easy.setOpt(new options::Url(getFullUri(uri)));
        setHttpMethodOpt(easy, httpMethod);
        easy.setOpt(new options::FollowLocation(true));
        easy.setOpt(new options::MaxRedirs(8));
        easy.setOpt(new options::WriteStream(&responseData));
        easy.setOpt(new options::SslCtxFunction(sslContextCallback));

        const auto& content  = postData.str();
        const auto contentLength = content.size();

        std::list<std::string> headers;
        headers.push_back("Content-Type: application/json");

        {
            std::stringstream ss;
            ss << "Content-Length: " << contentLength;
            headers.push_back(ss.str());
        }

        easy.setOpt(new options::HttpHeader(headers));
        easy.setOpt(new options::PostFields(content));
        easy.setOpt(new options::PostFieldSize(contentLength));
        easy.perform();

        m_httpStatus = infos::ResponseCode::get(easy);
        /*if (httpStatus == 200) {
            //throw std::runtime_error("");
            //std::cout << ss.str();
        }*/
    }
    catch (curlpp::LibcurlRuntimeError&) {
        //std::cout << ex.what() << std::endl;
        throw;
    }

    // Make sure we got something from the server
    const std::string& responseText = responseData.str();
    if (responseText.empty()) {
        throw std::runtime_error("Server sent an empty response");
    }

    // Parse response
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    if (!jsonReader.parse(responseText, jsonRoot)) {
        throw BadServerResponseException("Expected JSON format");
    }

    if (!jsonRoot.isMember("status") || !jsonRoot["status"].isInt()) {
        throw BadServerResponseException("Expected a status code");
    }

    int status = jsonRoot["status"].asInt();
    switch (status) {
    case 0:
        // Failure
        {
            std::string errorMsg("API call was unsuccessful.");
            if (jsonRoot.isMember("errors")) {
                Json::Value& jsonErrors = jsonRoot["errors"];
                if (!jsonErrors.isArray() || (jsonErrors.size() == 0)) {
                    throw BadServerResponseException("Expected details about what went wrong");
                }

                errorMsg += " Error(s):\n\n";
                for (auto it = jsonErrors.begin(), end(jsonErrors.end()); it != end; ++it) {
                    Json::Value& v = *it;
                    if (!v.isString() || v.asString().empty()) {
                        throw BadServerResponseException("Expected a non-empty string");
                    }

                    errorMsg += v.asString();
                    errorMsg += "\n";
                }
            }

            break;
        }

    case 1:
        // Success
        {
            break;
        }
    }

    jsonResponseData = jsonRoot;
}

void ApiController::Impl::ReceiveResponse(Json::Value& jsonResponseData) {
}

std::string ApiController::Impl::getFullUri(const std::string& uri) {
    std::string fullUri("http");
    if (m_useSsl) {
        fullUri += "s"; 
    }

    fullUri += "://";
    fullUri += m_host;
    fullUri += uri;
    return fullUri;
}

void ApiController::Impl::setHttpMethodOpt(curlpp::Easy& easy, const std::string& method) {
    using namespace curlpp;
#if 0
    std::string methodUc;
    std::transform(method.begin(), method.end(), methodUc.begin(), ::toupper);

    if (methodUc == "GET") {
        easy.setOpt(new options::HttpGet(true));
    }
    else if (methodUc == "POST") {
        easy.setOpt(new options::HttpPost(true));
    }
    else if (methodUc == "AUTH") {
        easy.setOpt(new options::HttpAuth(true));
    }
    else {
        throw std::logic_error("Unsupported HTTP method");
    }
#endif
}

//
// Public
//

ApiController::ApiController(const std::string& host) : m_impl(new Impl(host)) {}
ApiController::~ApiController() {}

void ApiController::sendRequest(const std::string& uri, const Json::Value& jsonRequestData, Json::Value& jsonResponseData,
    const std::string& httpMethod) {
    m_impl->SendRequest(uri, jsonRequestData, jsonResponseData, httpMethod);
}

void ApiController::receiveResponse(Json::Value& jsonResponseData) {
    m_impl->ReceiveResponse(jsonResponseData);
}

} } // namespace
