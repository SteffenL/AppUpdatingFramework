#include "core/progress/ProgressReaderWriter.h"
#include "core/progress/Product.h"
#include "core/exceptions.h"

#include <boost/filesystem.hpp>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>
#include <nowide/fstream.hpp>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestStream.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

void verify(aufw::progress::ProgressReaderWriterBase& progress, aufw::progress::Product& product) {
    using namespace aufw;
    using namespace aufw::progress;
    namespace fs = boost::filesystem;

    /*if (product.State !== State::VerifyPending) {
        return;
    }*/

    product.State = State::VerifyPending;

    try {
        if (!fs::exists(product.TempFilePath)) {
            throw FileException("File not found", product.TempFilePath);
        }

		nowide::ifstream stream(product.TempFilePath.c_str(), std::ios::binary);
        if (!stream.is_open()) {
            throw FileException("Cannot open file", product.TempFilePath);
        }

        const int readBufferSize = 128 * 1024;
        product.State = State::Verifying;
        Poco::SHA1Engine hashEngine;
        Poco::DigestOutputStream hashStream(hashEngine);

        // Read file and calculate hash
        std::unique_ptr<char> readBuffer(new char[readBufferSize]);
        do {
            stream.read(reinterpret_cast<char*>(readBuffer.get()), readBufferSize);
            std::streamsize bytesRead = stream.gcount();
            if (bytesRead > 0) {
                hashStream.write(readBuffer.get(), static_cast<size_t>(bytesRead));
            }
        } while (stream.good());

        // Finalize hash
        hashStream.flush();
        const Poco::DigestEngine::Digest& calculatedHashRaw = hashEngine.digest();
        std::string& calculatedHash = Poco::DigestEngine::digestToHex(calculatedHashRaw);

        // Verify
        std::string hash(product.UpdateDetails.UpdateFileHash);
        std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
        std::transform(calculatedHash.begin(), calculatedHash.end(), calculatedHash.begin(), ::tolower);

        if (calculatedHash.compare(product.UpdateDetails.UpdateFileHash) != 0) {
            product.State = State::VerifyFailed;
            return;
        }

        product.State = State::VerifyComplete;
    }
    catch (std::exception&) {
        product.State = State::VerifyFailed;
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

    if ((application.State >= State::DownloadComplete) && (application.State <= State::VerifyComplete)) {
        application.State = State::VerifyPending;

        try {
            verify(progressFile, application);
            nowide::cout << "... " << ((application.State == State::VerifyComplete) ? "OK" : "Failed") << std::endl;
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

        if ((component.State >= State::DownloadComplete) && (component.State <= State::VerifyComplete)) {
            component.State = State::VerifyPending;

            try {
                verify(progressFile, component);
                nowide::cout << "... " << ((component.State == State::VerifyComplete) ? "OK" : "Failed") << std::endl;
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