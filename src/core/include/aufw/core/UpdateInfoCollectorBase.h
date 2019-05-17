#ifndef __aufw_webapi__UpdateInfoCollectorBase__
#define __aufw_webapi__UpdateInfoCollectorBase__

#include <aufw/core/Version.h>
#include <aufw/core/ProductInstallationInfo.h>
#include <string>

namespace Json { class Value; }

namespace aufw {

class UpdateInfoCollectorBase {
public:
    UpdateInfoCollectorBase(const std::string& appName);
    virtual ~UpdateInfoCollectorBase();
    virtual void Collect() = 0;
    virtual void SetDefaultVersion(const Version& version);
    virtual void GetResult(ProductInstallationInfo& info) const;

protected:
    std::string m_appName;
    Version m_version;
    bool m_haveDefaultVersion;
    Version m_defaultVersion;
};

} // namespace
#endif // guard
