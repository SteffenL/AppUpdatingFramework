#ifndef __aufw_exceptions_msw__
#define __aufw_exceptions_msw__

#include <stdexcept>
#include <string>

namespace aufw {

class WindowsException : public std::runtime_error {
private:
    unsigned int m_code;

public:
    WindowsException(unsigned int code);
    std::string what();

private:
    std::string getMessageFromSystem();
};

} // namespace
#endif // guard
