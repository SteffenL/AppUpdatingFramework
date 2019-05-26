#include <string>
#include <stdexcept>

namespace aufw { namespace web_api {

struct BadServerResponseException : public std::runtime_error {
    BadServerResponseException(const std::string& message)
        : runtime_error(message) {}
};

} }
