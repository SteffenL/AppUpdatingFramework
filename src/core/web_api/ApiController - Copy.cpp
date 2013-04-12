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

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>
#include <json/json.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>

namespace aufw { namespace web_api {

//
// Implementation
//

class ApiController::Impl {
public:
    Impl(const std::string& host);
    void SendRequest(const std::string& uri, const Json::Value& jsonRequestData, const std::string& httpMethod);
    void ReceiveResponse(Json::Value& jsonResponseData);

private:
    static const int MAX_REQUEST_TRY_COUNT = 3;
    static const int MAX_RESPONSE_TRY_COUNT = 3;
    static const int MAX_RESPONSE_LENGTH_TO_RETRY =  2 * 1024 * 1024;
    Poco::Net::HTTPClientSession m_session;
};

ApiController::Impl::Impl(const std::string& host) : m_session(host) {
    // SSL
    /*const Context::Ptr context = new Context(Context::CLIENT_USE, std::string(), std::string(), std::string());
    HTTPSClientSession session(context);*/
    m_session.setTimeout(Poco::Timespan(5, 0));
}

void ApiController::Impl::SendRequest(const std::string& uri, const Json::Value& jsonRequestData, const std::string& httpMethod) {
    Json::FastWriter jsonWriter;
    const std::string& postData = jsonWriter.write(jsonRequestData);

    Poco::Net::HTTPRequest request(httpMethod, uri);
    request.setContentLength(postData.size());
    request.setContentType("application/json");

    bool isOk = false;
    std::string errorMsg;
    bool shouldContinue = false;
    int tryCount = 0;

    do {
        try {
            ++tryCount;
            m_session.sendRequest(request) << postData;
            isOk = true;
        }
        catch (Poco::Net::NetException& ex) {
            if (tryCount >= MAX_REQUEST_TRY_COUNT) {
                errorMsg = "Network error: ";
                errorMsg += ex.what();
            }
            else {
                shouldContinue = true;
            }
        }
    } while (shouldContinue);

    if (!isOk) {
        throw std::runtime_error(errorMsg.c_str());
    }
}

void ApiController::Impl::ReceiveResponse(Json::Value& jsonResponseData) {
    bool isOk = false;
    std::string errorMsg;
    bool shouldContinue = false;
    int tryCount = 0;
    std::ostringstream responseDataStream;

    do {
        std::streamsize bytesRead = 0;
        responseDataStream.clear();
        ++tryCount;

        try {
            Poco::Net::HTTPResponse response;
            std::istream& responseStream = m_session.receiveResponse(response);

            if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
                errorMsg = "Unsuccessful request to server. Reason: ";
                errorMsg += response.getReason();
                throw std::runtime_error(errorMsg.c_str());
            }

            const int bufferSize = 64 * 1024;
            std::unique_ptr<char> readBuffer(new char[bufferSize]);
            do
            {
                responseStream.read(readBuffer.get(), sizeof(readBuffer));
                bytesRead = responseStream.gcount();
                if (bytesRead > 0) {
                    responseDataStream.write(readBuffer.get(), bytesRead);
                }
            } while (responseStream.good() && (bytesRead > 0));
        }
        catch (Poco::Net::NetException& ex) {
            // If the file is large, don't try again (to save our precious bandwidth)
            if ((bytesRead >= MAX_RESPONSE_LENGTH_TO_RETRY) || (tryCount >= MAX_RESPONSE_TRY_COUNT)) {
                errorMsg = "Network error: ";
                errorMsg += ex.what();
                throw std::runtime_error(errorMsg.c_str());
            }
            else {
                shouldContinue = true;
            }
        }
    } while (shouldContinue);

    // Make sure we got something from the server
    std::string& responseData = responseDataStream.str();
    if (responseData.empty()) {
        throw std::runtime_error("Server sent an empty response");
    }

    // Parse response
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    if (!jsonReader.parse(responseData, jsonRoot)) {
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

//
// Public
//

ApiController::ApiController(const std::string& host) : m_impl(new Impl(host)) {}
ApiController::~ApiController() {}

void ApiController::sendRequest(const std::string& uri, const Json::Value& jsonRequestData, const std::string& httpMethod) {
    m_impl->SendRequest(uri, jsonRequestData, httpMethod);
}

void ApiController::receiveResponse(Json::Value& jsonResponseData) {
    m_impl->ReceiveResponse(jsonResponseData);
}

} } // namespace
