#include "AppCollector.h"
#include "FileVersionInfo.h"
#include <stdexcept>

namespace aufw {

AppCollector::AppCollector(const std::string& appName, const std::string& filePath) : UpdateInfoCollectorBase(appName), m_filePath(filePath) {}

void AppCollector::Collect() {
    FileVersionInfo fvi(m_filePath);
    if (!fvi.HasFileVersion) {
        throw std::runtime_error("File has no version info");
    }

    m_version = fvi.FileVersion;
}

} // namespace
