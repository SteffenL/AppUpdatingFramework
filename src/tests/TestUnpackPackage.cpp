#include "core/job/Job.h"
#include "core/package/FilePackage.h"
#include "core/job/MoveFilesStep.h"
#include "core/job/UnpackFilePackageStep.h"
#include "core/package/sources.h"

#include <UnitTest++.h>
#include <direct.h>

using namespace aufw;

TEST(UnpackPackage) {
    _chdir("../../../bin/Debug/");

    std::vector<std::string> packageFileNames;
    packageFileNames.push_back("package-7zip.zip");
    packageFileNames.push_back("package-winrar.zip");
    packageFileNames.push_back("package-winrar2.zip");

    for (auto& fileName : packageFileNames) {
        std::string filePath("testdata/packages/");
        filePath += fileName;
        package::FilePackage package(filePath);
        package::SourceBase& source = package.GetSource();

        std::vector<std::string> fileList;
        source.GetFileList(fileList);

        job::Job job;
        // Backup files
        job.AddStep(new job::MoveFilesStep(".", fileList, "update_backup"));
        // Unpack
        job.AddStep(new job::UnpackFilePackageStep(&package, "."));
        job.Execute();
        job.Rollback();
    }
}
