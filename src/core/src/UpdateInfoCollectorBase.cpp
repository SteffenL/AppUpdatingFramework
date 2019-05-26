#include <aufw/core/UpdateInfoCollectorBase.h>
#include <json/json.h>

namespace aufw {

UpdateInfoCollectorBase::UpdateInfoCollectorBase(const std::string& appName) : m_appName(appName), m_haveDefaultVersion(false) {}
UpdateInfoCollectorBase::~UpdateInfoCollectorBase() {}

void UpdateInfoCollectorBase::SetDefaultVersion(const Version& version) {
    m_defaultVersion = version;
    m_haveDefaultVersion = true;
}

void UpdateInfoCollectorBase::GetResult(ProductInstallationInfo& info) const {
    info.UniqueKey = m_appName;
    info.Version = m_version;
}

} // namespace
