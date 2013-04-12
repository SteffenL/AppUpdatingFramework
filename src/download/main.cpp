#include "core/progress/ProgressReaderWriter.h"
#include "core/progress/Product.h"
#include "core/exceptions.h"

#include "core/web_api/certificates/StartComCaCert.h"

#include <cyassl/ssl.h>
#include <curl/curl.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <boost/filesystem.hpp>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <fstream>

CURLcode sslContextCallback(void* ctx) {
    // Load CA certificate from memory
    aufw::web_api::certificates::StartComCaCert cert;
    const unsigned char* certData = cert.GetData();
    CyaSSL_CTX_load_verify_buffer(
        reinterpret_cast<CYASSL_CTX*>(ctx),
        certData,
        cert.GetSize(),
        SSL_FILETYPE_PEM);
    return CURLE_OK;
}

struct DownloadFileWriter
{
private:
    DownloadFileWriter();

public:
    DownloadFileWriter(std::ostream * stream) : mStream(stream), writeRound(0) {}

    // Helper Class for reading result from remote host
    size_t write(curlpp::Easy *handle, char* ptr, size_t size, size_t nmemb)
    {
        ++writeRound;

        curlpp::options::Url url;
        handle->getOpt(url);

        // Calculate the real size of the incoming buffer
        size_t realsize = size * nmemb;
        //std::cerr << "write round: " << writeRound << ", url: " << url.getValue() << std::endl;
        mStream->write(ptr, realsize);
        // return the real size of the buffer...
        return realsize;
    };

    // Public member vars
    std::ostream * mStream;
    unsigned writeRound;
};

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

    try {
        bool isOk = false;
        std::string errorMsg;
        bool shouldContinue = false;
        int tryCount = 0;


        product.State = State::Downloading;

        // Get response
        tryCount = 0;
        std::ofstream responseDataStream;
        DownloadDetails_t& downloadDetails = product.ProgressDetails.Download;

        do {
            std::streamsize bytesRead = 0;
            ++tryCount;
            shouldContinue = false;
            isOk = false;
            errorMsg.clear();

            try {
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

                    downloadDetails.FileSize = product.UpdateDetails.UpdateFileSize;//response.getContentLength64();
                    downloadDetails.TotalDownloaded = 0;
                    product.TempFilePath = tempFilePath.string();

                    responseDataStream.open(tempFilePath.string(), std::ios::binary);
                    if (!responseDataStream.is_open()) {
                        throw aufw::FileException("Unable to create/write to file", tempFilePath.string());
                    }
                }

                using namespace curlpp;
                Easy easy;
                easy.setOpt(new options::Url(product.UpdateDetails.UpdateDownloadUrl));
                //setHttpMethodOpt(easy, httpMethod);
                easy.setOpt(new options::FollowLocation(true));
                easy.setOpt(new options::MaxRedirs(8));
                easy.setOpt(new options::SslCtxFunction(sslContextCallback));
                easy.setOpt(new options::WriteStream(&responseDataStream));
                easy.perform();

                auto httpStatus = infos::ResponseCode::get(easy);
                if (httpStatus != 200) {
                    errorMsg = "Unsuccessful request to server. Reason: ";
                    //errorMsg += response.getReason();
                    // TODO: get reason
                    throw std::runtime_error(errorMsg.c_str());
                }


                /*product.State = State::Downloading;
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
                } while (responseStream.good() && (bytesRead > 0));*/

                // Check content length
                /*if (response.getContentLength64() != Poco::Net::HTTPMessage::UNKNOWN_CONTENT_LENGTH) {
                    if (downloadDetails.TotalDownloaded == 0) {
                        throw std::runtime_error("Empty file");
                    }

                    if (downloadDetails.TotalDownloaded != downloadDetails.FileSize) {
                        throw std::runtime_error("Total downloaded doesn't match total file size");
                    }
                }*/

                product.State = State::DownloadComplete;
                isOk = true;
            }
            catch (curlpp::LibcurlRuntimeError&) {
                // If the file is large, don't try again (to save our precious bandwidth)
                if ((bytesRead >= MAX_RESPONSE_LENGTH_TO_RETRY) || (tryCount >= MAX_RESPONSE_TRY_COUNT)) {
                    /*errorMsg = "Network error: ";
                    errorMsg += ex.what();
                    throw std::runtime_error(errorMsg.c_str());*/
                    throw;
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