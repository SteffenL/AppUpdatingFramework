#include "exceptions_msw.h"
#include <Poco/UnicodeConverter.h>
#include <Windows.h>
#include <sstream>

namespace aufw {

WindowsException::WindowsException(unsigned int code) : runtime_error(getMessageFromSystem()), m_code(code) {}

std::string WindowsException::what() {
    std::stringstream ret;
    ret << runtime_error::what() << " (Code: " << m_code << ")";
    return ret.str();
}

std::string WindowsException::getMessageFromSystem()
{
    LPWSTR msgW_c = nullptr;
    ::FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, m_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&msgW_c, 0, NULL);

    std::string msg;
    Poco::UnicodeConverter::toUTF8(msgW_c, msg);

    ::GlobalFree(msgW_c);

    return msg;
}

} // namespace
