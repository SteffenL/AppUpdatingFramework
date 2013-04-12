#include <string>
#include <stdexcept>

namespace aufw { namespace web_api {

struct BadServerResponseException : public std::runtime_error {
    BadServerResponseException(const std::string& message) : runtime_error(message.c_str()) {}
    std::string what() {
        std::string msg("Bad response from server\n");
        msg += runtime_error::what();
        return msg;
    }
};

} }
