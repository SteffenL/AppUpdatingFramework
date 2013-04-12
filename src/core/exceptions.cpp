#include "exceptions.h"
#include <sstream>

namespace aufw {

FileException::FileException(const std::string& message, const std::string& filePath)
    : std::runtime_error(message + ": " + filePath) {}

} // namespace

#ifdef _WIN32
#include "exceptions_msw.cpp"
#endif
