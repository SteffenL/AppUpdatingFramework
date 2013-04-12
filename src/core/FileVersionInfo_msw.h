#ifndef __aufw_FileVersionInfo_msw__
#define __aufw_FileVersionInfo_msw__

#include "FileVersionInfo.h"
#include <string>

namespace aufw {

class FileVersionInfo_msw : public FileVersionInfoBase {
public:
    FileVersionInfo_msw(const std::string& filePath);

private:
    void queryFileVersion(char* blockData);
};

typedef FileVersionInfo_msw FileVersionInfo;

} // namespace
#endif // guard
