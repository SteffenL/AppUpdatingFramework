#ifdef _WIN32
#include <aufw/core/StandardPaths.h>
#include "exceptions.h"

#include <Poco/UnicodeConverter.h>

#include <Windows.h>
#include <vector>

namespace aufw {

std::string StandardPaths::GetExecutablePath() {
    std::vector<wchar_t> buffer;
    unsigned int bufferLength = MAX_PATH;
    buffer.resize(bufferLength);

    bool isOk = false;

    do {
        DWORD requiredLength = ::GetModuleFileNameW(NULL, &buffer[0], buffer.size());

        // Failed?
        if (requiredLength == 0) {
            throw WindowsException(::GetLastError());
        }

        // If the returned module path was truncated, we need a larger buffer
        if (requiredLength >= bufferLength) {
            // Increase buffer length
            bufferLength <<= 1;
            buffer.resize(bufferLength);
        }

        isOk = true;
    } while (!isOk);

    std::string path;
    Poco::UnicodeConverter::toUTF8(&buffer[0], path);
    
    return path;
}

} // namespace
#endif
