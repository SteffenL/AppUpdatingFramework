#include "core/progress/ProgressReaderWriter.h"
#include "core/progress/Product.h"
#include "core/exceptions.h"

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/HTTPMessage.h>

#include <boost/filesystem.hpp>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <fstream>

void download(aufw::progress::ProgressReaderWriterBase& progress, aufw::progress::Product& product) {
    using namespace aufw::progress;
    namespace fs = boost::filesystem;
    
    /*if (product.State != State::DownloadPending) {
        return;
    }*/

    /*if ((product.State >= State::DownloadPending) && fs::exists(product.TempFilePath)) {
        product.State = State::DownloadComplete;
    }*/

    product.State = State::DownloadPending;

    static const int MAX_REQUEST_TRY_COUNT = 3;
    static const int MAX_RESPONSE_TRY_COUNT = 3;
    static const int MAX_RESPONSE_LENGTH_TO_RETRY =  2 * 1024 * 1024;
    Poco::Net::HTTPClientSession m_session;
    m_session.setTimeout(Poco::Timespan(5, 0));

    Poco::Net::HTTPRequest request("GET", product.UpdateDetails.UpdateDownloadUrl);

    try {
        bool isOk = false;
        std::string errorMsg;
        bool shouldContinue = false;
        int tryCount = 0;

        // Send request
        do {
            try {
                ++tryCount;
                m_session.sendRequest(request);
                isOk = true;
            }
            catch (Poco::Net::NetException& ex) {
                if (tryCount >= MAX_REQUEST_TRY_COUNT) {
                    errorMsg = "Network error: ";
                    errorMsg += ex.what();
                }
                else {
                    shouldContinue = true;
                }
            }
        } while (shouldContinue);

        if (!isOk) {
            throw std::runtime_error(errorMsg.c_str());
        }

        product.State = State::Downloading;

        // Get response
        isOk = false;
        errorMsg.clear();
        shouldContinue = false;
        tryCount = 0;
        std::ofstream responseDataStream;
        DownloadDetails_t& downloadDetails = product.ProgressDetails.Download;

        do {
            std::streamsize bytesRead = 0;
            ++tryCount;

            try {
                Poco::Net::HTTPResponse response;
                std::istream& responseStream = m_session.receiveResponse(response);

                if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
                    errorMsg = "Unsuccessful request to server. Reason: ";
                    errorMsg += response.getReason();
                    throw std::runtime_error(errorMsg.c_str());
                }

                if (!responseDataStream.is_open()) {
                    namespace fs = boost::filesystem;
                
                    fs::path tempFilePath(progress.DownloadDir);
                    tempFilePath /= "%%%%-%%%%-%%%%-%%%%";
                    tempFilePath = fs::unique_path(tempFilePath);

                    fs::path tempDir(tempFilePath.parent_path());
                    if (!fs::exists(tempDir)) {
                        if (!fs::create_directories(tempDir)) {
                            throw aufw::FileException("Unable to create directories", tempDir.string());
                        }
                    }

                    downloadDetails.FileSize = response.getContentLength64();
                    downloadDetails.TotalDownloaded = 0;
                    product.TempFilePath = tempFilePath.string();

                    responseDataStream.open(tempFilePath.string(), std::ios::binary);
                    if (!responseDataStream.is_open()) {
                        throw aufw::FileException("Unable to create/write to file", tempFilePath.string());
                    }
                }

                product.State = State::Downloading;
                const int bufferSize = 512 * 1024;
                std::unique_ptr<char> readBuffer(new char[bufferSize]);
                do
                {
                    responseStream.read(readBuffer.get(), bufferSize);
                    bytesRead = responseStream.gcount();
                    if (bytesRead > 0) {
                        responseDataStream.write(readBuffer.get(), bytesRead);
                        responseDataStream.flush();
                        downloadDetails.TotalDownloaded += bytesRead;
                    }
                } while (responseStream.good() && (bytesRead > 0));

                // Check content length
                if (response.getContentLength64() != Poco::Net::HTTPMessage::UNKNOWN_CONTENT_LENGTH) {
                    if (downloadDetails.TotalDownloaded == 0) {
                        throw std::runtime_error("Empty file");
                    }

                    if (downloadDetails.TotalDownloaded != downloadDetails.FileSize) {
                        throw std::runtime_error("Total downloaded doesn't match total file size");
                    }
                }

                product.State = State::DownloadComplete;
            }
            catch (Poco::Net::NetException& ex) {
                // If the file is large, don't try again (to save our precious bandwidth)
                if ((bytesRead >= MAX_RESPONSE_LENGTH_TO_RETRY) || (tryCount >= MAX_RESPONSE_TRY_COUNT)) {
                    errorMsg = "Network error: ";
                    errorMsg += ex.what();
                    throw std::runtime_error(errorMsg.c_str());
                }
                else {
                    shouldContinue = true;
                }
            }
        } while (shouldContinue);
    }
    catch (std::exception&) {
        product.State = State::DownloadFailed;
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
    if (application.State < State::DownloadComplete) {
        try {
            download(progressFile, application);
            nowide::cout << "... " << ((application.State == State::DownloadComplete) ? "OK" : "Failed") << std::endl;
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
        if (component.State < State::DownloadComplete) {
            try {
                download(progressFile, component);
                nowide::cout << "... " << ((application.State == State::DownloadComplete) ? "OK" : "Failed") << std::endl;
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