#include "FindUpdatesThread.h"

#include "core/web_api/v2/update/Update.h"
#include "core/AppCollector.h"
#include "core/StandardPaths.h"
#include "core/exceptions.h"

#include "core/progress/ProgressReaderWriter.h"
#include "core/progress/Product.h"

#include <Poco/Path.h>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>
#include <nowide/fstream.hpp>
#include <boost/filesystem.hpp>

#include <map>
#include <iostream>
#include <algorithm>

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

const wxEventType FindUpdatesThread::UpdateFoundEvent = wxNewEventType();
const wxEventType FindUpdatesThread::UpdateNotFoundEvent = wxNewEventType();
const wxEventType FindUpdatesThread::UpdateErrorEvent = wxNewEventType();
const wxEventType FindUpdatesThread::ServerErrorEvent = wxNewEventType();
const wxEventType FindUpdatesThread::ThreadExceptionEvent = wxNewEventType();

FindUpdatesThread::FindUpdatesThread(wxEvtHandler* parent, const std::string& appName, const std::string& vendorName)
    : m_parent(parent), m_appName(appName), m_vendorName(vendorName), wxThread() {}

void* FindUpdatesThread::Entry() {
    if (TestDestroy()) {
        return nullptr;
    }

    // Clone exception for re-throwing in the main thread
    try {
        checkNowInternal();
    }
    catch (...) {
        boost::exception_ptr* exception = new boost::exception_ptr(boost::current_exception());
        wxCommandEvent event(ThreadExceptionEvent);
        event.SetClientData(exception);
        wxPostEvent(m_parent, event);
    }

    return nullptr;
}

void FindUpdatesThread::OnExit() {}

void FindUpdatesThread::BeginCheck(wxEvtHandler* parent, const std::string& appName, const std::string& vendorName, OnCollect_t onCollect) {
    auto thread = new FindUpdatesThread(parent, appName, vendorName);
    if (thread->Create() != wxTHREAD_NO_ERROR) {
        return;
    }

    thread->OnCollect = onCollect;

    thread->Run();
}

void FindUpdatesThread::checkNowInternal() {
    namespace fs = boost::filesystem;
    using namespace aufw;
    using namespace web_api::v2;

    update::CheckArg checkArg;

    if (OnCollect) {
        OnCollect(checkArg.Application, checkArg.Components);
    }

    update::CheckResult checkResult;

    try {
        update::Update update;
        update.Check(checkArg, checkResult);
    }
    catch (std::runtime_error&) {
        /*nowide::cout << "A problem occurred while looking for updates:" << std::endl;
        nowide::cout << "    " << ex.what() << std::endl;*/
        throw;
    }

    if (!checkResult.HasApplicationUpdate && !checkResult.HaveComponentUpdates){
        // No updates found
        wxCommandEvent event(UpdateNotFoundEvent);
        wxPostEvent(m_parent, event);
        // No need to save progress
        return;
    }

    // Save initial progress
    fs::path exeDir(fs::absolute(StandardPaths::GetExecutablePath()).parent_path());
    fs::path tempDir(fs::absolute(Poco::Path::temp()).parent_path());

    fs::path updateBaseDir(tempDir);
    updateBaseDir /= m_vendorName;
    updateBaseDir /= m_appName;
    updateBaseDir /= "_update";
    updateBaseDir /= fs::unique_path();
    /*if (!fs::create_directories(updateBaseDir)) {
        throw FileException("Cannot create directories", updateBaseDir.string());
    }*/

    fs::path progressFilePath(updateBaseDir);
    progressFilePath /= "progress.xml";

    auto progressFile = new progress::ProgressReaderWriter(progressFilePath.string());
    auto& progressWriter = *progressFile;

    //fs::path exeDir(fs::path(StandardPaths::GetExecutablePath()).parent_path());
    fs::path targetDir(exeDir);
    fs::path backupDir(updateBaseDir);
    backupDir /= "backup";
    fs::path downloadDir(updateBaseDir);
    downloadDir /= "download";

    progressWriter.TargetDir = targetDir.string();
    progressWriter.BackupDir = backupDir.string();
    progressWriter.DownloadDir = downloadDir.string();

    if (checkResult.HasApplicationUpdate) {
        progress::Product application;
        application.State = progress::State::DownloadPending;
        application.UpdateDetails = checkResult.Application.UpdateDetails;
        application.InstalledVersion = checkArg.Application.Version;
        progressWriter.SetApplication(application);
    }

    //std::list<progress::Product> components;
    std::list<ProductUpdateDetails> componentDetailsList;
    checkResult.Components.GetUpdateDetails(componentDetailsList);
    std::for_each(componentDetailsList.begin(), componentDetailsList.end(), [&](const ProductUpdateDetails& details) {
        using namespace aufw;
        using namespace web_api::v2;
        progress::Product component;
        component.State = progress::State::DownloadPending;
        component.UpdateDetails = details;
        auto it = std::find_if(checkArg.Components.begin(), checkArg.Components.end(), [&](ProductInstallationInfo& ii) -> bool {
            return (ii.UniqueKey == component.UpdateDetails.UniqueKey);
        });
        if (it != checkArg.Components.end()) {
            component.InstalledVersion = it->Version;
        }
        //components.push_back(component);
        progressWriter.AddComponent(component);
    });

    // Don't save yet; wait until the user proceeds
    //progressWriter.Save();
    m_progressFilePath.assign(progressFilePath.string());

    wxCommandEvent event(UpdateFoundEvent);
    //event.SetString(m_progressFilePath);
    event.SetClientData(progressFile);
    wxPostEvent(m_parent, event);
}

std::string FindUpdatesThread::GetProgessFilePath() const {
    return m_progressFilePath;
}

} // namespace
