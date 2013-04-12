#include "core/job/Job.h"
#include "core/package/FilePackage.h"
#include "core/job/MoveFilesStep.h"
#include "core/job/UnpackFilePackageStep.h"
#include "core/package/sources.h"
#include "core/job/StartProgramStep.h"
#include "core/exceptions.h"
#include "core/StandardPaths.h"

#include <boost/filesystem.hpp>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>

#include <string>
#include <vector>
#include <stdexcept>

int mainStartup(int argc, char* argv[]) {
    namespace fs = boost::filesystem;
    using namespace aufw;

    fs::path exePath(StandardPaths::GetExecutablePath());

    nowide::cout << "To begin updating, run:" << std::endl;
    nowide::cout << exePath.filename().string() << " update <package file>" << std::endl;
    nowide::cout << "package file: Path to the update package file." << std::endl;

    return 0;
}

int updateStartup(int argc, char* argv[]) {
    namespace fs = boost::filesystem;
    using namespace aufw;

    if (argc <= 0) {
        throw std::invalid_argument("Path to update package file is missing");
    }

    fs::path exePath(StandardPaths::GetExecutablePath());
    fs::path packagePath(argv[0]);
    if (!fs::exists(packagePath)) {
        throw FileException("File not found", packagePath.string());
    }

    fs::path outputDir(exePath.parent_path());
    fs::path backupDir(exePath.parent_path());
    backupDir /= "_update";
    backupDir /= "backup";

    package::FilePackage package(packagePath.string());
    package::SourceBase& source = package.GetSource();

    std::vector<std::string> fileList;
    source.GetFileList(fileList);

    job::Job job;
    // Backup files
    job.AddStep(new job::MoveFilesStep(outputDir.string(), fileList, backupDir.string()));
    // Unpack
    job.AddStep(new job::UnpackFilePackageStep(package, outputDir.string()));
    // Restart app
    job.AddStep(new job::StartProgramStep(exePath.string()));
    job.Execute();

    return 0;
}

int main(int argc, char* argv[]) {
    nowide::args args(argc, argv);

    --argc;
    argv = &argv[1];

    std::string startupMode;
    if (argc >= 1) {
        startupMode = argv[0];
        --argc;
        argv = &argv[1];
    }

    int exitCode = 1;

    try {
        if (startupMode.empty()) {
            exitCode = mainStartup(argc, argv);
        }
        else if (startupMode == "update") {
            exitCode = updateStartup(argc, argv);
        }
        else {
            throw std::runtime_error("Invalid startup mode");
        }
    }
    catch (std::exception& ex) {
        nowide::cout << "Exception: " << ex.what() << std::endl;
        exitCode = 1;
    }

    return exitCode;
}