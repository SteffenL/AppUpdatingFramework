#include "PlainTextVersionCollector.h"
#include "FileVersionInfo.h"
#include "exceptions.h"
#include <json/json.h>
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <fstream>
#include <iostream>

namespace aufw {

PlainTextVersionCollector::PlainTextVersionCollector(const std::string& appName)
    : UpdateInfoCollectorBase(appName) {}

PlainTextVersionCollector::PlainTextVersionCollector(const std::string& appName, const std::string& filePath)
    : UpdateInfoCollectorBase(appName), m_filePath(filePath) {}

void PlainTextVersionCollector::Collect() {
    std::ifstream file(m_filePath);
    if (!file.is_open()) {
        if (m_haveDefaultVersion) {
            m_version = m_defaultVersion;
            return;
        }
        else {
            throw FileException("Could not open file", m_filePath);
        };
    }

    std::string versionStr;
    file >> versionStr;

    m_version = versionStr;
}

void PlainTextVersionCollector::setFilePath(const std::string& filePath) {
    m_filePath = filePath;
}

} // namespace
