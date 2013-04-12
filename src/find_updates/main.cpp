#include "AssetsVersionCollector.h"
#include "LanguagesVersionCollection.h"

#include "core/web_api/v2/update/Update.h"
#include "core/AppCollector.h"
#include "core/StandardPaths.h"
#include "core/exceptions.h"

#include "core/progress/ProgressWriter.h"
#include "core/progress/Product.h"

#include <Poco/Path.h>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>
#include <nowide/fstream.hpp>
#include <boost/filesystem.hpp>

#include <map>
#include <iostream>
#include <algorithm>

namespace aufw { namespace validation {

/*class UpdatesIntegrityValidator {
public:
    UpdatesIntegrityValidator(const std::string& hashesFilePath, const std::string& statusesFilePath);
};

class ProductUpdateStatusHashesFile {
public:
    void Load(const std::string& filePath);
    void Save();

protected:
    struct NameHashPair {
        std::string FilePath;
        std::string Hash;
    };

    std::vector<NameHashPair> m_hashes;
};

void ProductUpdateStatusHashesFile::Load(const std::string& filePath) {

}*/

} } // namespace

std::string g_exePath;

int mainStartup(int argc, char* argv[]) {
    /*if (argc <= 0) {
        throw std::invalid_argument("Path to update progress file is missing");
        return 1;
    }*/

    namespace fs = boost::filesystem;
    using namespace aufw;
    using namespace web_api::v2;

    update::CheckArg checkArg;

    // Application
    AppCollector appCollector("dise", StandardPaths::GetExecutablePath());
    appCollector.Collect();
    appCollector.GetResult(checkArg.Application);

    // Assets
    UpdateInfoCollection componentCollection;
    componentCollection.AddCollector(new updating::AssetsVersionCollector);
    componentCollection.Collect();
    componentCollection.GetResult(checkArg.Components);

    // Languages
    updating::LanguagesVersionCollection langCollection;
    langCollection.Collect();
    langCollection.GetResult(checkArg.Components);

    update::CheckResult checkResult;

    try {
        update::Update update;
        update.Check(checkArg, checkResult);
    }
    catch (std::runtime_error& ex) {
        nowide::cout << "A problem occurred while looking for updates:" << std::endl;
        nowide::cout << "    " << ex.what() << std::endl;
        return 1;
    }

    // Application update
    if (checkResult.HasApplicationUpdate) {
        const auto& application = checkResult.Application;
        nowide::cout << "Application update:" << std::endl;
        nowide::cout << "    " << checkResult.Application.UpdateDetails.DisplayName << " " << checkResult.Application.UpdateDetails.Version.ToString() << std::endl;
    }

    // Component updates
    if (checkResult.HaveComponentUpdates) {
        const auto& components = checkResult.Components;
        nowide::cout << "Component updates:" << std::endl;
        std::for_each(components.begin(), components.end(), [&](const update::CheckProductResult& component) {
            nowide::cout << "    " << component.UpdateDetails.DisplayName << " " << component.UpdateDetails.Version.ToString() << std::endl;
        });
    }

    // Save initial progress
    fs::path exeDir(fs::absolute(StandardPaths::GetExecutablePath()).parent_path());
    fs::path tempDir(fs::absolute(Poco::Path::temp()).parent_path());

    fs::path updateBaseDir(tempDir);
    updateBaseDir /= "CompanyName";
    updateBaseDir /= "ProductName";
    updateBaseDir /= "_update";
    updateBaseDir /= fs::unique_path();
    if (!fs::create_directories(updateBaseDir)) {
        throw FileException("Cannot create directories", updateBaseDir.string());
    }

    fs::path progressFilePath(updateBaseDir);
    progressFilePath /= "progress.xml";

    progress::ProgressWriter progressWriter(progressFilePath.string());

    //fs::path exeDir(fs::path(StandardPaths::GetExecutablePath()).parent_path());
    fs::path targetDir(exeDir);
    fs::path backupDir(updateBaseDir);
    backupDir /= "backup";
    fs::path downloadDir(updateBaseDir);
    downloadDir /= "download";

    progressWriter.TargetDir = targetDir.string();
    progressWriter.BackupDir = backupDir.string();
    progressWriter.DownloadDir = downloadDir.string();

    progress::Product application;
    application.State = progress::State::DownloadPending;
    application.UpdateDetails = checkResult.Application.UpdateDetails;
    progressWriter.SetApplication(application);

    std::list<progress::Product> components;
    std::list<ProductUpdateDetails> componentDetailsList;
    checkResult.Components.GetUpdateDetails(componentDetailsList);
    std::for_each(componentDetailsList.begin(), componentDetailsList.end(), [&](const ProductUpdateDetails& details) {
        using namespace aufw;
        using namespace web_api::v2;
        progress::Product component;
        component.State = progress::State::DownloadPending;
        component.UpdateDetails = details;
        progressWriter.AddComponent(component);
    });

    progressWriter.Save();
    nowide::cout << "Progress saved to " << progressFilePath.string() << std::endl;

    return 0;
}

int main(int argc, char* argv[]) {
    nowide::args args(argc, argv);

    g_exePath = argv[0];
    --argc;
    argv = &argv[1];

    int exitCode = 1;

    try {
        exitCode = mainStartup(argc, argv);
    }
    catch (std::exception& ex) {
        nowide::cout << "Exception: " << ex.what() << std::endl;
    }

    return exitCode;
}
