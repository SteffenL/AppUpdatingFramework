#ifndef __aufw_webapi__AppCollector__
#define __aufw_webapi__AppCollector__

#include <aufw/core/Version.h>
#include "UpdateInfoCollectorBase.h"
#include <string>

namespace aufw {

class AppCollector : public UpdateInfoCollectorBase {
public:
    AppCollector(const std::string& appName, const std::string& filePath);
    void Collect();

private:
    std::string m_filePath;
};

} // namespace
#endif // guard
