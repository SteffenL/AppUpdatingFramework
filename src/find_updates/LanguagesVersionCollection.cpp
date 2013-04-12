#include "LanguagesVersionCollection.h"
#include "core/StandardPaths.h"
#include "core/PlainTextVersionCollector.h"

#include <Poco/Format.h>
#include <boost/filesystem.hpp>

namespace updating {

LanguagesVersionCollection::LanguagesVersionCollection() {
    namespace fs = boost::filesystem;
    using namespace aufw;

    fs::path baseDir(StandardPaths::GetExecutablePath());
    baseDir.remove_filename();
    baseDir /= "lang";

    // No languages installed?
    if (!fs::exists(baseDir))
    {
        return;
    }

    // Collect installed languages
    for (fs::directory_iterator it(baseDir), end; it != end; ++it) {
        if (!fs::is_directory(it->status())) {
            continue;
        }

        const fs::path& langBaseDir = it->path();

        // Make sure there are files inside the folder
        if (fs::is_empty(langBaseDir)) {
            continue;
        }

        const std::string& langName = langBaseDir.filename().string();
        const std::string& ukey = Poco::format("dise-lang-%s", langName);
        fs::path versionFilePath(langBaseDir);
        versionFilePath /= "version.txt";

        PlainTextVersionCollector* versionCollector = new PlainTextVersionCollector(ukey, versionFilePath.string());

        if (!fs::exists(versionFilePath)) {
            versionCollector->SetDefaultVersion(Version(0, 0, 1, 0));
        };

        AddCollector(versionCollector);
    }
}

} // namespace
