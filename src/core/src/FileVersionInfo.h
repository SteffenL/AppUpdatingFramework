#ifndef __aufw_FileVersionInfo__
#define __aufw_FileVersionInfo__

#include <aufw/core/Version.h>
#include <string>

namespace aufw {

class FileVersionInfoBase
{
public:
    FileVersionInfoBase();

public:
    bool HasFileVersion;
    Version FileVersion;
};

} // namespace

#ifdef _WIN32
#include "FileVersionInfo_msw.h"
#else
#error Not implemented
#endif

#endif // guard
