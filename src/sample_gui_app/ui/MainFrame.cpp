#include "MainFrame.h"
#include "../app.h"
#include "core/FindUpdatesThread.h"
#include "ui/ui/FoundUpdatesDialog.h"

#include "core/StandardPaths.h"
#include "core/progress/ProgressReaderWriter.h"
#include "core/Process.h"

#include "core/AppCollector.h"
#include "find_updates/AssetsVersionCollector.h"
#include "find_updates/LanguagesVersionCollection.h"

#include <wx/msgdlg.h>
#include <wx/config.h>
#include <boost/exception_ptr.hpp>
#include <Poco/Process.h>

namespace ui {

BEGIN_EVENT_TABLE(MainFrame, MainFrameBase)
    EVT_COMMAND(wxID_ANY, aufw::FindUpdatesThread::UpdateFoundEvent, MainFrame::OnUpdateFound)
    EVT_COMMAND(wxID_ANY, aufw::FindUpdatesThread::UpdateNotFoundEvent, MainFrame::OnUpdateNotFound)
    EVT_COMMAND(wxID_ANY, aufw::FindUpdatesThread::UpdateErrorEvent, MainFrame::OnUpdateError)
    EVT_COMMAND(wxID_ANY, aufw::FindUpdatesThread::ServerErrorEvent, MainFrame::OnServerError)
    EVT_COMMAND(wxID_ANY, aufw::FindUpdatesThread::ThreadExceptionEvent, MainFrame::OnThreadException)
END_EVENT_TABLE()

MainFrame::MainFrame(wxWindow* parent) : MainFrameBase(parent), m_shouldBlockCheckForUpdates(false) {}

void MainFrame::checkForUpdatesOnButtonClick(wxCommandEvent& event) {
    m_shouldBlockCheckForUpdates = true;
    m_checkForUpdates->Enable(!m_shouldBlockCheckForUpdates);

    // Load progress file path
    wxString progressFilePath;
    wxConfig config(APP_NAME, VENDOR_NAME);
    if (config.Read("Update/ProgressFile", &progressFilePath)) {
        if (wxFileExists(progressFilePath)) {
            auto& progressFilePathUtf8 = progressFilePath.ToUTF8();
            auto progressFile = new aufw::progress::ProgressReaderWriter(
                std::string(progressFilePathUtf8.data(), progressFilePathUtf8.data() + progressFilePathUtf8.length()));
            progressFile->Load();

            if (progressFile->IsReadyToInstall()) {
                aufw::ui::FoundUpdatesDialog dlg(this, progressFile, APP_NAME, VENDOR_NAME, false);
                dlg.ShowModal();
                if (dlg.IsRestartNeeded()) {
                    Close(true);
                    return;
                }

                m_shouldBlockCheckForUpdates = false;
                m_checkForUpdates->Enable(!m_shouldBlockCheckForUpdates);
                return;
            }
        }
    }

    auto onCollect = [](aufw::ProductInstallationInfo& application, std::list<aufw::ProductInstallationInfo>& components) {
        using namespace aufw;

        // Application
        AppCollector appCollector("dise", StandardPaths::GetExecutablePath());
        appCollector.Collect();
        appCollector.GetResult(application);

        // Assets
        UpdateInfoCollection componentCollection;
        componentCollection.AddCollector(new updating::AssetsVersionCollector);
        componentCollection.Collect();
        componentCollection.GetResult(components);

        // Languages
        updating::LanguagesVersionCollection langCollection;
        langCollection.Collect();
        langCollection.GetResult(components);
    };

    aufw::FindUpdatesThread::BeginCheck(this, APP_NAME, VENDOR_NAME, onCollect);
}

void MainFrame::OnUpdateFound(wxCommandEvent& event) {
    auto progressFile = static_cast<aufw::progress::ProgressReaderWriter*>(event.GetClientData());

    auto dlg = new aufw::ui::FoundUpdatesDialog(this, progressFile, APP_NAME, VENDOR_NAME, false);
    Bind(aufw::ui::EVT_RESTART_REQUIRED, &MainFrame::OnRestartRequired, this);
    dlg->ShowModal();
    if (dlg->IsRestartNeeded()) {
        if (dlg->IsInstallComplete()) {
            restartApp(false);
        }

        Close(true);
        return;
    }

    m_shouldBlockCheckForUpdates = false;
    m_checkForUpdates->Enable(!m_shouldBlockCheckForUpdates);
}

void MainFrame::OnUpdateNotFound(wxCommandEvent& event) {
    m_shouldBlockCheckForUpdates = false;
    m_checkForUpdates->Enable(!m_shouldBlockCheckForUpdates);
}

void MainFrame::OnUpdateError(wxCommandEvent& event) {}

void MainFrame::OnServerError(wxCommandEvent& event) {}

void MainFrame::OnThreadException(wxCommandEvent& event) {
    boost::exception_ptr* exceptionPtr = static_cast<boost::exception_ptr*>(event.GetClientData());
    assert(exceptionPtr);
    // Clone exception
    boost::exception_ptr exception(*exceptionPtr);
    // Delete original exception object
    delete exceptionPtr;

    if (exception) {
        try {
            boost::rethrow_exception(exception);
        }
        catch (std::exception& ex) {
            wxMessageBox(ex.what(), wxEmptyString, wxOK | wxICON_ERROR, this);
        }
    }

    m_shouldBlockCheckForUpdates = false;
    m_checkForUpdates->Enable(!m_shouldBlockCheckForUpdates);
}

void MainFrame::restartApp(bool continueInstallUpdates, bool elevate, bool& veto) {
    // Launch new instance of app
    std::string& exePath = aufw::StandardPaths::GetExecutablePath();
    std::string params;
    if (continueInstallUpdates) {
        params = "-ContinueInstallUpdates";
    }

    if (elevate) {
        if (!aufw::Process::LaunchElevated(exePath, params, this->GetHandle())) {
            veto = true;
            return;
        }
    } else {
        aufw::Process::Launch(exePath, params);
    }
}

void MainFrame::restartApp(bool elevate) {
    bool veto;
    restartApp(false, elevate, veto);
}

void MainFrame::OnRestartRequired(aufw::ui::RestartRequiredEvent& event) {
    bool veto = false;
    //IsInstallComplete()
    restartApp(event.GetContinueInstallUpdates(), event.IsElevationNeeded(), veto);
    if (!veto) {
        return;
    }

    event.Veto();
}

} // namespace
