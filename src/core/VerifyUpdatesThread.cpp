#include "VerifyUpdatesThread.h"

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

#include "core/progress/ProgressReaderWriter.h"
#include "core/progress/Product.h"
#include "core/exceptions.h"

#include "core/web_api/certificates/StartComCaCert.h"

#include <Poco/SHA1Engine.h>
#include <Poco/DigestStream.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <boost/filesystem.hpp>

// Exceptions
#include <boost/exception_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdexcept>

namespace aufw {

const wxEventType VerifyUpdatesThread::StateChangedEvent = wxNewEventType();
const wxEventType VerifyUpdatesThread::VerifyCompleteEvent = wxNewEventType();
const wxEventType VerifyUpdatesThread::VerifyFailedEvent = wxNewEventType();
const wxEventType VerifyUpdatesThread::UpdateErrorEvent = wxNewEventType();
const wxEventType VerifyUpdatesThread::ServerErrorEvent = wxNewEventType();
const wxEventType VerifyUpdatesThread::ThreadExceptionEvent = wxNewEventType();
const wxEventType VerifyUpdatesThread::ThreadExitEvent = wxNewEventType();

struct VerifyFailedException : public std::runtime_error {};

VerifyUpdatesThread::VerifyUpdatesThread(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback)
    : m_parent(parent), m_progressFile(progressFile), m_cancelCallback(cancelCallback), m_wasCanceled(false), wxThread() {}

void* VerifyUpdatesThread::Entry() {
    if (TestDestroy()) {
        return nullptr;
    }

    // Clone exception for re-throwing in the main thread
    try {
        verifyNowInternal();
    }
    catch (...) {
        boost::exception_ptr* exception = new boost::exception_ptr(boost::current_exception());
        wxCommandEvent event(ThreadExceptionEvent);
        event.SetClientData(exception);
        wxPostEvent(m_parent, event);
    }

    return nullptr;
}

void VerifyUpdatesThread::OnExit() {
    wxCommandEvent event(ThreadExitEvent);
    wxPostEvent(m_parent, event);
}

void VerifyUpdatesThread::BeginVerify(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback) {
    auto thread = new VerifyUpdatesThread(parent, progressFile, cancelCallback);
    if (thread->Create() != wxTHREAD_NO_ERROR) {
        return;
    }

    thread->Run();
}

void VerifyUpdatesThread::verifyNowInternal() {
    using namespace aufw::progress;

    auto& application = m_progressFile.GetApplicationWritable();

    if (m_progressFile.HasApplication())
    {
        if ((application.State >= State::DownloadComplete) && (application.State <= State::VerifyComplete)) {
            application.State = State::VerifyPending;
            m_progressFile.Save();

            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&application));
            wxPostEvent(m_parent, event);

            try {
                verify(application);
            }
            catch (std::exception&) {
                throw;
            }

            m_progressFile.Save();
        }
        else {
        }
    }

    auto& components = m_progressFile.GetComponentsWritable();
    std::for_each(components.begin(), components.end(), [&](Product& component) {
        using namespace aufw::progress;

        if ((component.State >= State::DownloadComplete) && (component.State <= State::VerifyComplete)) {
            component.State = State::VerifyPending;
            m_progressFile.Save();

            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&component));
            wxPostEvent(m_parent, event);

            try {
                verify(component);
            }
            catch (std::exception&) {
                throw;
            }

            m_progressFile.Save();
        }
        else {
        }
    });

    m_progressFile.Save();

    // Check if all are complete
    if (!m_progressFile.HasApplication() || (application.State >= State::VerifyComplete)) {
        bool isComplete = true;
        std::for_each(components.begin(), components.end(), [&](Product& component) {
            using namespace aufw::progress;
            if (component.State < State::VerifyComplete) {
                isComplete = false;
                return;
            }
        });

        if (isComplete) {
            wxCommandEvent event(VerifyCompleteEvent);
            wxPostEvent(m_parent, event);
            return;
        }
    }

    // Check if at least one failed
    bool isFailed = false;
    if (m_progressFile.HasApplication() && (application.State == State::VerifyFailed)) {
        isFailed = true;
        wxCommandEvent event(VerifyFailedEvent);
        wxPostEvent(m_parent, event);
        return;
    }

    std::for_each(components.begin(), components.end(), [&](Product& component) {
        using namespace aufw::progress;
        if (component.State == State::VerifyFailed) {
            isFailed = true;
            return;
        }
    });

    if (isFailed) {
        wxCommandEvent event(VerifyFailedEvent);
        wxPostEvent(m_parent, event);
        return;
    }
}

void VerifyUpdatesThread::verify(aufw::progress::Product& product) {
    using namespace aufw;
    using namespace aufw::progress;
    namespace fs = boost::filesystem;
    
    /*if (product.State != State::DownloadPending) {
        return;
    }*/

    /*if ((product.State >= State::DownloadPending) && fs::exists(product.TempFilePath)) {
        product.State = State::DownloadComplete;
    }*/

    product.State = State::VerifyPending;
    m_progressFile.Save();

    {
        wxCommandEvent event(StateChangedEvent);
        event.SetClientData(static_cast<void*>(&product));
        wxPostEvent(m_parent, event);
    }

    try {
        if (!fs::exists(product.TempFilePath)) {
            throw FileException("File not found", product.TempFilePath);
        }

        std::ifstream stream(product.TempFilePath, std::ios::binary);
        if (!stream.is_open()) {
            throw FileException("Cannot open file", product.TempFilePath);
        }

        const int readBufferSize = 128 * 1024;
        product.State = State::Verifying;
        m_progressFile.Save();

        wxCommandEvent event(StateChangedEvent);
        event.SetClientData(static_cast<void*>(&product));
        wxPostEvent(m_parent, event);

        Poco::SHA1Engine hashEngine;
        Poco::DigestOutputStream hashStream(hashEngine);

        // Read file and calculate hash
        std::unique_ptr<char> readBuffer(new char[readBufferSize]);
        do {
            if (testCanceled()) {
                product.State = State::VerifyPending;
                m_progressFile.Save();

                {
                    wxCommandEvent event(StateChangedEvent);
                    event.SetClientData(static_cast<void*>(&product));
                    wxPostEvent(m_parent, event);
                }

                return;
            }

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
            m_progressFile.Save();

            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&product));
            wxPostEvent(m_parent, event);

            return;
        }

        product.State = State::VerifyComplete;
        m_progressFile.Save();

        {
            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&product));
            wxPostEvent(m_parent, event);
        }
    }
    catch (std::exception&) {
        product.State = State::VerifyFailed;
        m_progressFile.Save();

        wxCommandEvent event(StateChangedEvent);
        event.SetClientData(static_cast<void*>(&product));
        wxPostEvent(m_parent, event);

        throw;
    }
}

bool VerifyUpdatesThread::testCanceled() {
    m_wasCanceled = (m_cancelCallback ? m_cancelCallback() : false);
    return m_wasCanceled;
}

} // namespace
