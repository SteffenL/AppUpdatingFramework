#include "app.h"
#include "ui/MainFrame.h"
#include "ui/ui/FoundUpdatesDialog.h"
#include "core/StandardPaths.h"
#include "core/Process.h"
#include "core/Elevation.h"
#include "core/progress/ProgressReaderWriter.h"
#include "core/BoostFileSystemBugWorkaround.h"
#include <wx/config.h>
#include <wx/debug.h>
#include <wx/cmdline.h>
#include <wx/log.h>
#include <memory>
#include <Poco/ScopedLock.h>
#include <Poco/NamedMutex.h>
#include <Poco/Exception.h>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>

// Leak detection
#ifdef _DEBUG
#include <vld.h>
#endif

IMPLEMENT_APP(MyApp)

MyApp::MyApp() : m_exitCode(0) {}

bool MyApp::OnInit() {
    m_startupMode = StartupMode::Main;

    if (!wxApp::OnInit()) {
        return false;
    }

    // Setup locale for UTF-8
    std::locale::global(boost::locale::generator().generate(""));
    boost::filesystem::path::imbue(std::locale());

    try {
        // Create a mutex so that the this process will wait for the parent process to exit
        Poco::NamedMutex mutex("BD5B9403-95A3-4789-8801-DA56F034EBAA");
        Poco::ScopedLock<Poco::NamedMutex> mutexLock(mutex);
    }
    catch (Poco::SystemException&) {
        // Ignore exception thrown when mutex was abandoned
        // Sadly, other exceptions will also be catched
    }

    // Load progress file path
    wxString progressFilePath;
    {
        wxConfig config(APP_NAME, VENDOR_NAME);
        if (config.Read("Update/ProgressFile", &progressFilePath)) {
            if (wxFileExists(progressFilePath)) {
                if (m_startupMode != StartupMode::ContinueInstallUpdates) {
                    m_startupMode = StartupMode::InstallUpdates;
                }
            }
            else {
                config.DeleteEntry("Update/ProgressFile");
            }
        }
    }

    StartupMode::type currentStartupMode;
    bool startupIsSuccess;

    do {
        currentStartupMode = m_startupMode;
        startupIsSuccess = false;

        switch (currentStartupMode) {
        case StartupMode::Main:
            startupIsSuccess = mainStartup();
            break;

        case StartupMode::InstallUpdates:
        case StartupMode::ContinueInstallUpdates:
            startupIsSuccess = updateStartup(progressFilePath, (m_startupMode == StartupMode::ContinueInstallUpdates));
            break;

        default:
            wxFAIL_MSG("Invalid startup mode");
            break;
        }
    } while (currentStartupMode != m_startupMode); // Allow changing startup mode

    return startupIsSuccess;
}

int MyApp::OnExit() {
    return m_exitCode;
}

void MyApp::OnInitCmdLine(wxCmdLineParser& parser) {
    parser.SetSwitchChars(wxT("-"));
    parser.SetCmdLine(wxApp::argc, wxApp::argv);
    parser.AddSwitch(wxT("ContinueInstallUpdates"), wxEmptyString, wxEmptyString, wxCMD_LINE_VAL_NONE);
}

bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser) {
    if (parser.Found(wxT("ContinueInstallUpdates"))) {
        m_startupMode = StartupMode::ContinueInstallUpdates;
    }

    return true;
}

bool MyApp::mainStartup() {
    ui::MainFrame* frame = new ui::MainFrame(nullptr);
    frame->Show();
    return true;
}

bool MyApp::updateStartup(const wxString& progressFilePath, bool autoStartInstall) {
    if (progressFilePath.IsEmpty() || !wxFileExists(progressFilePath)) {
        wxLogError("Path to progress file is empty, or the file is missing.");
        return false;
    }

    auto& progressFilePathUtf8 = progressFilePath.ToUTF8();
    using aufw::progress::ProgressReaderWriter;
    auto progressFile = new ProgressReaderWriter(std::string(progressFilePathUtf8.data(), progressFilePathUtf8.data() + progressFilePathUtf8.length()));

    // Try to load the progress file
    try {
        progressFile->Load();
    }
    catch (std::exception&) {
        wxMessageBox(wxString::Format(_("Cannot install updates\n\nThe progress file appears to be damaged:\n%s"), progressFilePath), _("Error"), wxOK | wxICON_ERROR);
        // Cleanup and start normally
        progressFile->CleanupFiles();
        m_startupMode = StartupMode::Main;
        return false;
    }

    // If installation was completed, no need to continue
    if (progressFile->InstallIsComplete()) {
        // Cleanup and start normally
        progressFile->CleanupFiles();
        m_startupMode = StartupMode::Main;
        return false;
    }

    class DummyFrame : public wxTopLevelWindow {
    public:
        wxWindow* m_fakeParent;

        DummyFrame() : wxTopLevelWindow(nullptr, wxID_ANY, wxEmptyString) {
            Bind(aufw::ui::EVT_RESTART_REQUIRED, &DummyFrame::OnRestartRequired, this);
        }

        void OnRestartRequired(aufw::ui::RestartRequiredEvent& event) {
            bool veto = false;
            //IsInstallComplete()
            MyApp::restartApp(event.GetContinueInstallUpdates(), event.IsElevationNeeded(), veto, m_fakeParent->GetHandle());
            if (!veto) {
                return;
            }

            event.Veto();
        }

        void SetFakeParent(wxWindow* window) {
            m_fakeParent = window;
        }
    };

    auto dummyFrame = new DummyFrame();
    aufw::ui::FoundUpdatesDialog dlg(dummyFrame, progressFile, APP_NAME, VENDOR_NAME, autoStartInstall);
    dummyFrame->SetFakeParent(&dlg);

    dlg.CenterOnScreen();
    dlg.ShowModal();

    if (dlg.IsRestartNeeded()) {
        if (dlg.IsInstallComplete()) {
            restartApp(false, dlg.GetHandle());
        }
    }

    dummyFrame->Close(true);
    return false;
}

void MyApp::restartApp(bool continueInstallUpdates, bool elevate, bool& veto, void* parentWindow) {
    // Launch new instance of app
    std::string& exePath = aufw::StandardPaths::GetExecutablePath();
    std::string params;
    if (continueInstallUpdates) {
        params = "-ContinueInstallUpdates";
    }

    // Create a mutex so that the new process can wait for this one to exit
    Poco::NamedMutex mutex("BD5B9403-95A3-4789-8801-DA56F034EBAA");
    mutex.lock();

    if (elevate && !aufw::Elevation::IsUserAdmin()) {
        if (!aufw::Process::LaunchElevated(exePath, params, parentWindow)) {
            veto = true;
            return;
        }
    } else {
        if (aufw::Elevation::IsUserAdmin()) {
            if (!aufw::Process::LaunchNonElevated(exePath, params)) {
                aufw::Process::Launch(exePath, params);
            }
        }
        else {
            aufw::Process::Launch(exePath, params);
        }
    }
}

void MyApp::restartApp(bool elevate, void* parentWindow) {
    bool veto;
    restartApp(false, elevate, veto, parentWindow);
}
