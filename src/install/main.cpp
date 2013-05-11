#include "core/progress/ProgressReaderWriter.h"
#include "core/progress/Product.h"
#include "core/exceptions.h"
#include "core/package/FilePackage.h"
#include "core/package/sources.h"
#include "core/job/Job.h"
#include "core/job/MoveFilesStep.h"
#include "core/job/UnpackFilePackageStep.h"
#include "core/job/StartProgramStep.h"

#include <boost/filesystem.hpp>
#include <nowide/args.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

void install(aufw::progress::ProgressReaderWriterBase& progress, aufw::progress::Product& product) {
    using namespace aufw;
    using namespace aufw::progress;
    namespace fs = boost::filesystem;

    try {
        if (product.State >= State::InstallComplete) {
            throw std::runtime_error("Already installed");
        }

        product.State = State::InstallPending;

        if (!fs::exists(product.TempFilePath)) {
            throw FileException("File not found", product.TempFilePath);
        }

        package::FilePackage package(product.TempFilePath);
        package::SourceBase& source = package.GetSource();

        std::vector<std::string> fileList;
        source.GetFileList(fileList);

        job::Job job;
        bool needPrivileges = false;
        job.OnExecutionFailed = [&needPrivileges](const job::JobFailureArg& arg) {
            using namespace aufw;
            //nowide::cout << std::endl << "Installation failed. " << arg.Exception.what() << std::endl;
            if (dynamic_cast<FileException*>(arg.Exception)) {
                // Probably need more privileges to do anything with files
                needPrivileges = true;
            }
            else {
                nowide::cout << std::endl << "Exception: " << (arg.Exception ? arg.Exception->what() : "(unknown)") << std::endl;
            }
        };

        job.OnRollbackFailed = [](const job::JobFailureArg& arg) {
            //nowide::cout << std::endl << "Rollback failed. " << arg.Exception.what() << std::endl;
        };

        /*job.OnExecutionSuccess = []() {

        };

        job.OnRollbackSuccess = []() {

        };*/

        // Backup files
        job.AddStep(new job::MoveFilesStep(progress.TargetDir, fileList, progress.BackupDir));
        // Unpack
        job.AddStep(new job::UnpackFilePackageStep(package, progress.TargetDir));
        // Restart app
        //job.AddStep(new job::StartProgramStep(exePath.string()));
        if (!job.Execute()) {
            throw std::runtime_error("Job failed to complete");
        }

        product.State = State::InstallComplete;
    }
    catch (std::exception&) {
        product.State = State::InstallFailed;
        throw;
    }
}

int mainStartup(int argc, char* argv[]) {
    namespace fs = boost::filesystem;
    using namespace aufw;
    using namespace aufw::progress;

    if (argc <= 0) {
        throw std::invalid_argument("Path to update progress file is missing");
    }

    fs::path progressFilePath(argv[0]);
    if (!fs::exists(progressFilePath)) {
        throw FileException("File not found", progressFilePath.string());
    }

    ProgressReaderWriter progressFile(progressFilePath.string());
    progressFile.Load();

    nowide::cout << "===== Application =====" << std::endl;
    auto& application = progressFile.GetApplicationWritable();
    nowide::cout << application.UpdateDetails.DisplayName;

    if ((application.State >= State::VerifyComplete) && (application.State < State::InstallComplete)) {
        application.State = State::InstallPending;

        try {
            install(progressFile, application);
            nowide::cout << "... " << ((application.State == State::InstallComplete) ? "OK" : "Failed") << std::endl;
        }
        catch (std::exception& ex) {
            nowide::cout << std::endl << "Exception: " << ex.what() << std::endl;
        }

        progressFile.Save();
    }
    else {
        nowide::cout << "... " << "Skipped" << std::endl;
    }

    nowide::cout << "===== Components =====" << std::endl;
    auto& components = progressFile.GetComponentsWritable();
    std::for_each(components.begin(), components.end(), [&](Product& component) {
        using namespace aufw::progress;
        nowide::cout << component.UpdateDetails.DisplayName;

        if ((component.State >= State::VerifyComplete) && (component.State < State::InstallComplete)) {
            component.State = State::InstallPending;

            try {
                install(progressFile, component);
                nowide::cout << "... " << ((component.State == State::InstallComplete) ? "OK" : "Failed") << std::endl;
            }
            catch (std::exception& ex) {
                nowide::cout << std::endl << "Exception: " << ex.what() << std::endl;
            }

            progressFile.Save();
        }
        else {
            nowide::cout << "... " << "Skipped" << std::endl;
        }
    });

    progressFile.Save();

    return 0;
}

int main(int argc, char* argv[]) {
    nowide::args args(argc, argv);

    --argc;
    argv = &argv[1];

    int exitCode = 1;

    try {
        exitCode = mainStartup(argc, argv);
    }
    catch (std::exception& ex) {
        nowide::cout << std::endl << "Exception: " << ex.what() << std::endl;
    }

    return exitCode;
}