#include "core/job/Job.h"
#include "core/package/FilePackage.h"
#include "core/job/MoveFilesStep.h"
#include "core/job/UnpackFilePackageStep.h"
#include "core/package/sources.h"

#include <UnitTest++.h>

using namespace aufw;

TEST(UnpackPackage) {
    package::FilePackage package("package.zip");
    package::SourceBase& source = package.GetSource();

    std::vector<std::string> fileList;
    source.GetFileList(fileList);

    job::Job job;
    // Backup files
    job.AddStep(new job::MoveFilesStep(".", fileList, "update_backup"));
    // Unpack
    job.AddStep(new job::UnpackFilePackageStep(package, "."));
    job.Execute();
    job.Rollback();
}
