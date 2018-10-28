#ifndef __aufw_webapi_ApiController__
#define __aufw_webapi_ApiController__

#include <string>
#include <memory>

namespace Json { class Value; }

namespace aufw { namespace web_api {

class ApiController {
public:
    ApiController(const std::string& host = "www.steffenl.com");
    virtual ~ApiController();

protected:
    void sendRequest(const std::string& uri, const Json::Value& jsonRequestData, Json::Value& jsonResponseData,
        const std::string& httpMethod = "GET");

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} } // namespace
#endif // guard
