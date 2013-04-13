#ifndef __aufw_exceptions__
#define __aufw_exceptions__

#include <string>
#include <stdexcept>
#include <memory>

namespace aufw {

class FileException : public std::runtime_error {
public:
    FileException(const std::string& message, const std::string& filePath);
    FileException(const std::string& message, const std::string& filePath, const std::string& filePath2);
};

} // namespace

#ifdef _WIN32
#include "exceptions_msw.h"
#endif

#endif // guard
