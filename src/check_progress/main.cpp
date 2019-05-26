#include "DetailsPrinter.h"

#include <aufw/core/progress/ProgressReader.h>
#include <aufw/core/progress/Product.h>

#include <boost/filesystem.hpp>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

std::string g_exePath;

int mainStartup(int argc, char* argv[]) {
    if (argc <= 0) {
        throw std::invalid_argument("Path to update progress file is missing");
        return 1;
    }

    namespace fs = boost::filesystem;
    fs::path progressFilePath(argv[0]);

    using namespace aufw::progress;
    ProgressReader progressReader(progressFilePath.string());
    progressReader.Load();

    nowide::cout << "Target dir: " << progressReader.TargetDir << std::endl;
    nowide::cout << "Backup dir: " << progressReader.BackupDir << std::endl;
    nowide::cout << "Download dir: " << progressReader.DownloadDir << std::endl;
    nowide::cout << std::endl;

    nowide::cout << "===== Application =====" << std::endl;
    const auto& application = progressReader.GetApplication();
    DetailsPrinter::Current(application);

    nowide::cout << "===== Components =====" << std::endl;
    const auto& components = progressReader.GetComponents();
    std::for_each(components.begin(), components.end(), [&](const Product& product) {
        DetailsPrinter::Current(product);
    });

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
        nowide::cout << std::endl << "Exception: " << ex.what() << std::endl;
    }

    return exitCode;
}