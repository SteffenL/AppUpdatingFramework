#include "DownloadUpdatesThread.h"

/*
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
#include <algorithm>*/

#include <aufw/core/progress/ProgressReaderWriter.h>
#include <aufw/core/progress/Product.h>
#include "exceptions.h"

#include <curl/curl.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include <iostream>
#include <boost/filesystem.hpp>
#include <nowide/fstream.hpp>
#include <cmath>

// Exceptions
#include <boost/exception_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdexcept>

/*
class server_error : public std::runtime_error {
public:
    server_error(const std::string& _Message) : std::runtime_error(_Message) {}
    server_error(const char* _Message) : std::runtime_error(_Message) {}
};*/

#if 0
class wxProtocolErrorTranslator {
public:
    static wxString ToString(/*wxProtocolError error*/);
};

wxString wxProtocolErrorTranslator::ToString(/*wxProtocolError error*/) {
    wxString text;

    switch (error) {
    case wxPROTO_NOERR:
        text = wxT("No error");
        break;

    case wxPROTO_NETERR:
        text = wxT("A generic network error occurred");
        break;

    case wxPROTO_PROTERR:
        text = wxT("An error occurred during negotiation");
        break;

    case wxPROTO_CONNERR:
        text = wxT("The client failed to connect the server");
        break;

    case wxPROTO_INVVAL:
        text = wxT("Invalid value");
        break;

        //case wxPROTO_NOHNDLR:

    case wxPROTO_NOFILE:
        text = wxT("The remote file doesn't exist");
        break;

    case wxPROTO_ABRT:
        text = wxT("Last action aborted");
        break;

    case wxPROTO_RCNCT:
        text = wxT("An error occurred during reconnection");
        break;

    case wxPROTO_STREAMING:
        text = wxT("Someone tried to send a command during a transfer");
        break;

    default:
        wxASSERT(false);
    }

    return text;
}
#endif

namespace aufw {

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

class ProgressCallback {
public:
    ProgressCallback(wxEvtHandler* parent, DownloadUpdatesThread& thread) : m_parent(parent), m_thread(thread), m_lastProgress(-1) {}

    double operator()(double dltotal, double dlnow, double ultotal, double ulnow) {
        if (m_thread.TestCanceled()) {
            return CURL_READFUNC_ABORT;
        }

        double progress = (dlnow == 0) ? 0 : ((dlnow / dltotal) * 100);
        //float percent = std::floorf(progress * 100) / 100;
        int percent = std::floor(progress);

        if (percent == m_lastProgress) {
            return 0;
        }

        m_lastProgress = percent;
        wxCommandEvent event(m_thread.DownloadProgressEvent);
        event.SetInt(percent);
		wxQueueEvent(m_parent, event.Clone());
 
        return 0;
    }

private:
    wxEvtHandler* m_parent;
    DownloadUpdatesThread& m_thread;
    int m_lastProgress;
};

const wxEventType DownloadUpdatesThread::StateChangedEvent = wxNewEventType();
const wxEventType DownloadUpdatesThread::DownloadsCompleteEvent = wxNewEventType();
const wxEventType DownloadUpdatesThread::DownloadProgressEvent = wxNewEventType();
const wxEventType DownloadUpdatesThread::DownloadFailedEvent = wxNewEventType();
const wxEventType DownloadUpdatesThread::UpdateErrorEvent = wxNewEventType();
const wxEventType DownloadUpdatesThread::ServerErrorEvent = wxNewEventType();
const wxEventType DownloadUpdatesThread::ThreadExceptionEvent = wxNewEventType();
const wxEventType DownloadUpdatesThread::ThreadExitEvent = wxNewEventType();

DownloadUpdatesThread::DownloadUpdatesThread(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback)
    : m_parent(parent), m_progressFile(progressFile), m_cancelCallback(cancelCallback), m_wasCanceled(false), wxThread() {}

void* DownloadUpdatesThread::Entry() {
    if (TestDestroy()) {
        return nullptr;
    }

    // Clone exception for re-throwing in the main thread
    try {
        downloadNowInternal();
    }
    catch (...) {
        boost::exception_ptr* exception = new boost::exception_ptr(boost::current_exception());
        wxCommandEvent event(ThreadExceptionEvent);
        event.SetClientData(exception);
        wxQueueEvent(m_parent, event.Clone());
    }

    return nullptr;
}

void DownloadUpdatesThread::OnExit() {
    wxCommandEvent event(ThreadExitEvent);
    wxQueueEvent(m_parent, event.Clone());
}

void DownloadUpdatesThread::BeginDownload(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback) {
    auto thread = new DownloadUpdatesThread(parent, progressFile, cancelCallback);
    if (thread->Create() != wxTHREAD_NO_ERROR) {
        return;
    }

    thread->Run();
}

void DownloadUpdatesThread::downloadNowInternal() {
    using namespace aufw::progress;

	auto& application = m_progressFile.GetApplicationWritable();
    if (m_progressFile.HasApplication())
    {
        //nowide::cout << application.UpdateDetails.DisplayName;
        if ((application.State < State::DownloadComplete) || (application.State == State::VerifyFailed)) {
            try {
                download(application);
                //nowide::cout << "... " << ((application.State == State::DownloadComplete) ? "OK" : "Failed") << std::endl;
            }
            catch (curlpp::LibcurlRuntimeError&) {}
            catch (std::exception&) {
                //nowide::cout << std::endl << "Exception: " << ex.what() << std::endl;
                throw;
            }

            //m_progressFile.Save();
        }
        else {
            //nowide::cout << "... " << "Skipped" << std::endl;
        }
    }

    //nowide::cout << "===== Components =====" << std::endl;
    auto& components = m_progressFile.GetComponentsWritable();
    std::for_each(components.begin(), components.end(), [&](Product& component) {
        using namespace aufw::progress;
        //nowide::cout << component.UpdateDetails.DisplayName;
        if ((component.State < State::DownloadComplete) || (component.State == State::VerifyFailed)) {
            try {
                download(component);
                //nowide::cout << "... " << ((application.State == State::DownloadComplete) ? "OK" : "Failed") << std::endl;
            }
            catch (curlpp::LibcurlRuntimeError&) {}
            catch (std::exception&) {
                //nowide::cout << std::endl << "Exception: " << ex.what() << std::endl;
                throw;
            }

            //m_progressFile.Save();
        }
        else {
            //nowide::cout << "... " << "Skipped" << std::endl;
        }
    });

    //m_progressFile.Save();

    // Check if all are complete
    if (!m_progressFile.HasApplication() || (application.State >= State::DownloadComplete)) {
        bool isComplete = true;
        std::for_each(components.begin(), components.end(), [&](Product& component) {
            using namespace aufw::progress;
            if (component.State < State::DownloadComplete) {
                isComplete = false;
                return;
            }
        });

        if (isComplete) {
            wxCommandEvent event(DownloadsCompleteEvent);
			wxQueueEvent(m_parent, event.Clone());
        }
    }

    // Check if at least one failed
    bool isFailed = false;
    if (m_progressFile.HasApplication() && (application.State == State::DownloadFailed)) {
        isFailed = true;
        wxCommandEvent event(DownloadFailedEvent);
		wxQueueEvent(m_parent, event.Clone());
        return;
    }

    std::for_each(components.begin(), components.end(), [&](Product& component) {
        using namespace aufw::progress;
        if (component.State == State::DownloadFailed) {
            isFailed = true;
            return;
        }
    });

    if (isFailed) {
        wxCommandEvent event(DownloadFailedEvent);
        wxQueueEvent(m_parent, event.Clone());
        return;
    }
}

void DownloadUpdatesThread::download(aufw::progress::Product& product) {
    using namespace aufw::progress;
    namespace fs = boost::filesystem;
    
    /*if (product.State != State::DownloadPending) {
        return;
    }*/

    /*if ((product.State >= State::DownloadPending) && fs::exists(product.TempFilePath)) {
        product.State = State::DownloadComplete;
    }*/

    product.State = State::DownloadPending;
    m_progressFile.Save();

    wxCommandEvent event(StateChangedEvent);
    event.SetClientData(static_cast<void*>(&product));
	wxQueueEvent(m_parent, event.Clone());

    static const int MAX_REQUEST_TRY_COUNT = 1;
    static const int MAX_RESPONSE_TRY_COUNT = 1;
    static const int MAX_RESPONSE_LENGTH_TO_RETRY =  2 * 1024 * 1024;

    try {
        bool isOk = false;
        std::string errorMsg;
        bool shouldContinue = false;
        int tryCount = 0;


        product.State = State::Downloading;
        m_progressFile.Save();

        wxCommandEvent event(StateChangedEvent);
        event.SetClientData(static_cast<void*>(&product));
		wxQueueEvent(m_parent, event.Clone());

        // Get response
        tryCount = 0;
        nowide::ofstream responseDataStream;
        DownloadDetails_t& downloadDetails = product.ProgressDetails.Download;

        do {
            if (TestCanceled()) {
                product.State = State::DownloadPending;
                m_progressFile.Save();

                wxCommandEvent event(StateChangedEvent);
                event.SetClientData(static_cast<void*>(&product));
				wxQueueEvent(m_parent, event.Clone());
                return;
            }

            std::streamsize bytesRead = 0;
            ++tryCount;
            shouldContinue = false;
            isOk = false;
            errorMsg.clear();

            try {
                if (!responseDataStream.is_open()) {
                    namespace fs = boost::filesystem;
                
                    fs::path tempFilePath(m_progressFile.DownloadDir);
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

                    responseDataStream.open(tempFilePath.string().c_str(), std::ios::binary);
                    if (!responseDataStream.is_open()) {
                        throw aufw::FileException("Unable to create/write to file", tempFilePath.string());
                    }
                }

                using namespace curlpp;
                Easy easy;
                easy.setOpt(new options::Url(product.UpdateDetails.UpdateDownloadUrl));
                //setHttpMethodOpt(easy, httpMethod);
                easy.setOpt(new options::ConnectTimeout(5));
                easy.setOpt(new options::FollowLocation(true));
                easy.setOpt(new options::MaxRedirs(8));
                easy.setOpt(new options::WriteStream(&responseDataStream));
                easy.setOpt(new options::NoProgress(0));
                ProgressCallback pcb(m_parent, *this);
                //utilspp::BindFirst(utilspp::make_functor(ProgressCallback), )
                easy.setOpt(new options::ProgressFunction(pcb));
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
                m_progressFile.Save();

                wxCommandEvent event(StateChangedEvent);
                event.SetClientData(static_cast<void*>(&product));
				wxQueueEvent(m_parent, event.Clone());

                isOk = true;
            }
            catch (curlpp::LibcurlRuntimeError& ex) {
                if (ex.whatCode() == CURLE_ABORTED_BY_CALLBACK) {
                    break;
                }

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
        m_progressFile.Save();

        wxCommandEvent event(StateChangedEvent);
        event.SetClientData(static_cast<void*>(&product));
		wxQueueEvent(m_parent, event.Clone());

        throw;
    }
}

bool DownloadUpdatesThread::TestCanceled() {
    m_wasCanceled = (m_cancelCallback ? m_cancelCallback() : false);
    return m_wasCanceled;
}

} // namespace
