#ifndef __aufw_ProductInstallationInfo__
#define __aufw_ProductInstallationInfo__

#include <aufw/core/Version.h>
#include <string>

namespace aufw {

struct ProductInstallationInfo {
    std::string UniqueKey;
    Version Version;
};

} // namespace
#endif// guard
