#include "AssetsVersionCollector.h"
#include "core/StandardPaths.h"
#include <boost/filesystem.hpp>

namespace updating {

AssetsVersionCollector::AssetsVersionCollector() : aufw::PlainTextVersionCollector("di-assets") {}

void AssetsVersionCollector::Collect() {
    namespace fs = boost::filesystem;
    using namespace aufw;

    fs::path baseDir(StandardPaths::GetExecutablePath());
    baseDir.remove_filename();
    baseDir /= "assets";

    fs::path versionFilePath(baseDir);
    versionFilePath /= "version.txt";

    fs::path uiDir(baseDir);
    uiDir /= "ui";

    // Not installed?
    if (!fs::exists(versionFilePath) && !fs::exists(uiDir)) {
        return;
    }

    // If version.txt exists, read the version string; else, assume the version.
    // This is to make old versions compatible with the current updating system
    if (fs::exists(versionFilePath)) {
        setFilePath(versionFilePath.string());
        PlainTextVersionCollector::Collect();
    }
    else {
        m_version = 0x00000100;
    }
};

} // namespace
