#include "FoundUpdatesDialog.h"
#include "core/FindUpdatesThread.h"
#include "core/DownloadUpdatesThread.h"
#include "core/VerifyUpdatesThread.h"
#include "core/InstallUpdatesThread.h"

#include "core/progress/ProgressReaderWriter.h"
#include "core/progress/Product.h"
#include "core/progress/State.h"
#include "core/Process.h"
#include "core/Elevation.h"

#include <wx/webview.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>
#include <wx/config.h>

#include <boost/exception_ptr.hpp>
#include <algorithm>

#ifdef __WXMSW__
// Needed for Button_SetElevationRequiredState
#include <CommCtrl.h>

#include <Windows.h>
#include <Uxtheme.h>
#include <wx/msw/winundef.h>
#endif

namespace aufw { namespace ui {

wxDEFINE_EVENT(EVT_RESTART_REQUIRED, RestartRequiredEvent);

RestartRequiredEvent::RestartRequiredEvent()
    : wxCommandEvent(EVT_RESTART_REQUIRED), m_elevationNeeded(false), m_veto(false), m_canVeto(true), m_continueInstallUpdates(false) {}
wxEvent* RestartRequiredEvent::Clone() const { return new RestartRequiredEvent(*this); }
bool RestartRequiredEvent::IsElevationNeeded() const { return m_elevationNeeded; }
void RestartRequiredEvent::SetElevationNeeded(bool v) { m_elevationNeeded = v; }
bool RestartRequiredEvent::CanVeto() const { return m_canVeto; }
void RestartRequiredEvent::SetCanVeto(bool v) { m_veto = v; }
void RestartRequiredEvent::Veto(bool v) { m_veto = v; }
bool RestartRequiredEvent::GetVeto() const { return m_canVeto && m_veto; }
bool RestartRequiredEvent::GetContinueInstallUpdates() const { return m_continueInstallUpdates; }
void RestartRequiredEvent::SetContinueInstallUpdates(bool v) { m_continueInstallUpdates = v; }

BEGIN_EVENT_TABLE(FoundUpdatesDialog, FoundUpdatesDialogBase)
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::StateChangedEvent, FoundUpdatesDialog::OnStateChanged)
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::DownloadsCompleteEvent, FoundUpdatesDialog::OnDownloadsComplete)
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::DownloadProgressEvent, FoundUpdatesDialog::OnDownloadProgress)
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::DownloadFailedEvent, FoundUpdatesDialog::OnDownloadFailed)
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::ThreadExitEvent, FoundUpdatesDialog::OnDownloadThreadExit)
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::ThreadExceptionEvent, FoundUpdatesDialog::OnDownloadThreadException)

    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::StateChangedEvent, FoundUpdatesDialog::OnStateChanged)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::VerifyCompleteEvent, FoundUpdatesDialog::OnVerifyComplete)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::VerifyFailedEvent, FoundUpdatesDialog::OnVerifyFailed)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::ThreadExitEvent, FoundUpdatesDialog::OnVerifyThreadExit)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::ThreadExceptionEvent, FoundUpdatesDialog::OnThreadException)

    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::StateChangedEvent, FoundUpdatesDialog::OnStateChanged)
    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::InstallCompleteEvent, FoundUpdatesDialog::OnInstallComplete)
    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::ThreadExceptionEvent, FoundUpdatesDialog::OnInstallThreadException)

	/*EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::UpdateErrorEvent, FoundUpdatesDialog::OnUpdateError)
	EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::ServerErrorEvent, FoundUpdatesDialog::OnServerError)*/

END_EVENT_TABLE()

FoundUpdatesDialog::FoundUpdatesDialog(wxWindow* parent, aufw::progress::ProgressReaderWriter* progressFile,
    const wxString& appName, const wxString& vendorName, bool autoStartInstall)
    : FoundUpdatesDialogBase(parent),
    m_progressFile(progressFile),
    m_appName(appName),
    m_vendorName(vendorName),
    m_isInstalling(false),
    m_autoStartInstall(autoStartInstall),
    m_isRestartRequired(false),
    m_isInstallComplete(false),
    m_isWorking(false),
    m_shouldStopUpdating(false),
    m_shouldDeleteUpdates(false) {}

FoundUpdatesDialog::~FoundUpdatesDialog() {
    if (m_shouldDeleteUpdates) {
        deleteUpdates();
    }
}

void FoundUpdatesDialog::OnInitDialog(wxInitDialogEvent& event) {
    //m_progressFile->Load();
    m_appRestartText->SetLabel(wxString::Format(_("%s will restart automatically after installation."), m_appName));
    m_isReadyToInstall = m_progressFile->IsReadyToInstall();
    hideAllPanels();
    setupPanels();
    setupProductList();
    // Add web view
    m_releaseNotes = wxWebView::New(m_releaseNotesContainer, wxID_ANY);
    m_releaseNotesContainer->GetSizer()->Add(m_releaseNotes, wxSizerFlags(1).Expand());
    m_releaseNotesContainer->Layout();
    // Select the first item in the list to load the URL
    if (m_products->GetItemCount() > 0) {
        m_products->SetFocus();
        m_products->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }

    if (m_autoStartInstall) {
        installNowOnButtonClick(wxCommandEvent());
    }
}

void FoundUpdatesDialog::updateNowOnButtonClick(wxCommandEvent& event) {
    m_updateNow->Enable(false);
    m_shouldStopUpdating = false;
    // Store initial progress
    m_progressFile->Save();
    // Save progress file path
    {
        wxConfig config(m_appName, m_vendorName);
        config.Write("Update/ProgressFile", wxString(m_progressFile->GetFilePath()));
    }

    beginDownload();
}

void FoundUpdatesDialog::skipUpdateOnButtonClick(wxCommandEvent& event) {
    // TODO: Implement skipUpdateOnButtonClick
}

void FoundUpdatesDialog::dontUpdateOnButtonClick(wxCommandEvent& event) {
    auto button = static_cast<wxButton*>(event.GetEventObject());
    button->Enable(false);
/*
    wxRichMessageDialog dialog(
        this,
        _("Updates are not yet installed\n\nYou'll be asked again next time unless you delete them."),
        _("Update"),
        wxOK | wxCANCEL | wxCENTER);
    //dialog.ShowDetailedText(_("Delaying installation isn't recommended"))
    dialog.ShowCheckBox(_("Delete updates"));
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }*/

    if (m_isWorking)
    {
        stopUpdating();
    }
    else {
        Close();
    }

/*
    if (dialog.IsCheckBoxChecked()) {
        m_shouldDeleteUpdates = true;
    }*/
}

void FoundUpdatesDialog::productsOnListItemSelected(wxListEvent& event) {
    auto& product = *reinterpret_cast<aufw::progress::Product*>(event.GetItem().GetData());
    if (!product.UpdateDetails.ReleaseNotesUrl.empty()) {
        m_releaseNotes->LoadURL(wxString::FromUTF8(product.UpdateDetails.ReleaseNotesUrl.c_str()));
    }
    else {
        m_releaseNotes->LoadURL("about:blank");
    }
}

void FoundUpdatesDialog::addApplication(aufw::progress::Product& product) {
    using aufw::progress::State;
    wxListItem item;
    auto itemIndex = m_products->GetItemCount();
    item.SetText(wxString::FromUTF8(product.UpdateDetails.DisplayName.c_str()));
    item.SetData(static_cast<void*>(&product));
    item.SetId(itemIndex);
    itemIndex = m_products->InsertItem(item);
    m_productListIndexMap[&product] = itemIndex;
    m_products->SetItem(itemIndex, 1, wxString::FromUTF8(product.UpdateDetails.Version.ToString().c_str()));
    m_products->SetItem(itemIndex, 2, product.InstalledVersion.ToString());
    m_products->SetItem(itemIndex, 3, wxString::FromUTF8(State::GetStateText(product.State).c_str()));
}

void FoundUpdatesDialog::addComponent(aufw::progress::Product& product) {
    using aufw::progress::State;
    wxListItem item;
    auto itemIndex = m_products->GetItemCount();
    item.SetText(wxString::FromUTF8(product.UpdateDetails.DisplayName.c_str()));
    item.SetData(static_cast<void*>(&product));
    item.SetId(itemIndex);
    itemIndex = m_products->InsertItem(item);
    m_productListIndexMap[&product] = itemIndex;
    m_products->SetItem(itemIndex, 1, wxString::FromUTF8(product.UpdateDetails.Version.ToString().c_str()));
    m_products->SetItem(itemIndex, 2, product.InstalledVersion.ToString());
    m_products->SetItem(itemIndex, 3, wxString::FromUTF8(State::GetStateText(product.State).c_str()));
}

void FoundUpdatesDialog::moreInfoOnMenuSelection(wxCommandEvent& event) {
    // Get selected item index
    auto index = m_products->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index == -1) {
        return;
    }

    auto& product = *reinterpret_cast<aufw::progress::Product*>(m_products->GetItemData(index));
    wxLaunchDefaultBrowser(product.UpdateDetails.InfoUrl);
}

void FoundUpdatesDialog::moreInfoOnUpdateUI(wxUpdateUIEvent& event) {
    // Get selected item index
    auto index = m_products->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index == -1) {
        event.Enable(false);
        return;
    }

    // Enable menu item only if we have the URL
    auto& product = *reinterpret_cast<aufw::progress::Product*>(m_products->GetItemData(index));
    event.Enable(!product.UpdateDetails.InfoUrl.empty());
}

void FoundUpdatesDialog::manualDownloadOnMenuSelection(wxCommandEvent& event) {
    // Get selected item index
    auto index = m_products->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index == -1) {
        return;
    }

    auto& product = *reinterpret_cast<aufw::progress::Product*>(m_products->GetItemData(index));
    wxLaunchDefaultBrowser(product.UpdateDetails.ManualDownloadUrl);
}

void FoundUpdatesDialog::manualDownloadOnUpdateUI(wxUpdateUIEvent& event) {
    // Get selected item index
    auto index = m_products->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index == -1) {
        event.Enable(false);
        return;
    }

    // Enable menu item only if we have the URL
    auto& product = *reinterpret_cast<aufw::progress::Product*>(m_products->GetItemData(index));
    event.Enable(!product.UpdateDetails.ManualDownloadUrl.empty());
}

void FoundUpdatesDialog::OnStateChanged(wxCommandEvent& event) {
    using namespace aufw::progress;
    // Update UI
    auto& product = *static_cast<Product*>(event.GetClientData());
    auto itemIndex = m_productListIndexMap[&product];
    m_products->SetItem(itemIndex, 3, wxString::FromUTF8(State::GetStateText(product.State).c_str()));
}

void FoundUpdatesDialog::OnDownloadsComplete(wxCommandEvent& event) {
    Freeze();
    m_downloadProgressPanel->Hide();
    Layout();
    Refresh();
    Update();
    Thaw();

    auto cancelCallback = [&]() -> bool {
        return m_shouldStopUpdating;
    };

    VerifyUpdatesThread::BeginVerify(this, *m_progressFile, cancelCallback);
}

void FoundUpdatesDialog::OnVerifyComplete(wxCommandEvent& event) {
    bool isReady = m_progressFile->IsReadyToInstall();
    if (m_isReadyToInstall != isReady) {
        m_isReadyToInstall = isReady;
        m_isInstalling = true;
        hideAllPanels();
        setupPanels();

        auto cancelCallback = [&]() -> bool {
            return m_shouldStopUpdating;
        };

        InstallUpdatesThread::BeginInstall(this, *m_progressFile, cancelCallback);
    }
}

void FoundUpdatesDialog::OnVerifyFailed(wxCommandEvent& event) {
    m_verifyHasFailed = true;
    setWorkingState(false);
}

void FoundUpdatesDialog::OnVerifyThreadExit(wxCommandEvent& event) {
    if (m_verifyHasFailed) {
        if (wxMessageBox(_("One or more downloaded files are damaged\n\nWould you like to retry?"), _("Integrity check failed"), wxYES_NO | wxICON_QUESTION) == wxYES) {
            using namespace aufw::progress;

            if (m_progressFile->HasApplication()) {
                auto& application = m_progressFile->GetApplicationWritable();
                if (application.State == State::VerifyFailed) {
                    application.State = State::DownloadPending;
                }
            }

            if (m_progressFile->HaveComponents()) {
                auto& components = m_progressFile->GetComponentsWritable();
                std::for_each(components.begin(), components.end(), [&](Product& component) {
                    using namespace aufw::progress;
                    if (component.State == State::VerifyFailed) {
                        component.State = State::DownloadPending;
                    }
                });
            }

            beginDownload();
        }
    }
    else if (m_shouldStopUpdating) {

    }
}

void FoundUpdatesDialog::OnInstallComplete(wxCommandEvent& event) {
    setWorkingState(false);
    m_isInstalling = false;
    // Cleanup
    m_progressFile->CleanupFiles();
    // Notify that restart is required
    m_isInstallComplete = true;
    m_isRestartRequired = true;
    m_isElevationNeeded = false;
    Close(true);
}

void FoundUpdatesDialog::OnDownloadThreadException(wxCommandEvent& event) {
    OnThreadException(event);
}

void FoundUpdatesDialog::OnInstallThreadException(wxCommandEvent& event) {
    if (aufw::Elevation::IsUserAdmin()) {
        OnThreadException(event);
    }
    else {
        // Notify that restart is required (elevated)
        RestartRequiredEvent e;
        e.SetElevationNeeded(true);
        e.SetContinueInstallUpdates(true);
        bool canClose = true;
        if (GetParent()->GetEventHandler()->ProcessEvent(e)) {
            if (e.GetVeto()) {
                canClose = false;
                m_isInstalling = false;
                hideAllPanels();
                setupPanels();
            }
        }

        setWorkingState(false);

        if (canClose) {
            m_isInstallComplete = false;
            m_isRestartRequired = true;
            m_isElevationNeeded = true;
            Close(true);
        }
        else {
            m_isInstallComplete = false;
            m_isRestartRequired = false;
            m_isElevationNeeded = false;
        }
    }
}

void FoundUpdatesDialog::OnDownloadProgress(wxCommandEvent& event) {
    m_downloadProgress->SetValue(event.GetInt());
}

void FoundUpdatesDialog::OnDownloadFailed(wxCommandEvent& event) {
    m_downloadHasFailed = true;
    setWorkingState(false);
}

void FoundUpdatesDialog::OnUpdateError(wxCommandEvent& event) {}

void FoundUpdatesDialog::OnServerError(wxCommandEvent& event) {}

void FoundUpdatesDialog::OnThreadException(wxCommandEvent& event) {
    boost::exception_ptr* exceptionPtr = static_cast<boost::exception_ptr*>(event.GetClientData());
    assert(exceptionPtr);
    // Clone exception
    boost::exception_ptr exception(*exceptionPtr);
    // Delete original exception object
    delete exceptionPtr;

    if (exception)
    {
        try {
            boost::rethrow_exception(exception);
        }
        catch (std::exception& ex) {
            wxMessageBox(ex.what(), wxEmptyString, wxOK | wxICON_ERROR, this);
        }
    }
}

/*
void FoundUpdatesDialog::OnJobStepProgress(aufw::job::StepProgressArg& arg) {
    
}*/

void FoundUpdatesDialog::installNowOnButtonClick(wxCommandEvent& event) {
    setWorkingState(true);
    Freeze();
    m_installButtonsPanel->Hide();
    Layout();
    Refresh();
    Update();
    Thaw();
    m_isInstalling = true;
    m_shouldStopUpdating = false;

    auto cancelCallback = [&]() -> bool {
        return m_shouldStopUpdating;
    };

    InstallUpdatesThread::BeginInstall(this, *m_progressFile, cancelCallback);
}

void FoundUpdatesDialog::dontInstallOnButtonClick(wxCommandEvent& event) {
    m_isRestartRequired = false;
    Close();
}

void FoundUpdatesDialog::hideAllPanels() {
    auto& children = this->GetChildren();
    auto it = children.begin();
    while (it != children.end()) {
        wxWindow* window = *it;
        window->Hide();
        ++it;
    }
    /*std::for_each(children.begin(), children.end(), [&](wxWindow* window) {
        window->Hide();
    });*/
}

void FoundUpdatesDialog::setupPanels() {
    using namespace aufw;

    Freeze();

    if (m_isReadyToInstall) {
        m_headerText->SetLabel(_("Updates are ready to be installed."));

#ifdef __WXMSW__
        // Set shield icon on button if needed
        if (!Elevation::IsUserAdmin()) {
            Button_SetElevationRequiredState(static_cast<HWND>(m_installNow->GetHandle()), true);
        }
#else
#error Not implemented
#endif

        if (!m_isInstalling) {
            m_installButtonsPanel->Show();
        }
    }
    else {
        m_headerText->SetLabel(_("An update has been released!"));

#ifdef __WXMSW__
        // Set shield icon on button
        if (!Elevation::IsUserAdmin()) {
            Button_SetElevationRequiredState(static_cast<HWND>(m_updateNow->GetHandle()), true);
        }
#else
#error Not implemented
#endif

        m_updateButtonsPanel->Show();
    }

    m_headerPanel->Show();
    m_releaseNotesContainer->Show();
    m_productsPanel->Show();
    m_footerPanel->Show();

    Layout();
    Refresh();
    Update();
    Thaw();
}

void FoundUpdatesDialog::setupProductList() {
    using namespace aufw::progress;

    // Insert columns
    m_products->InsertColumn(0, wxT("Product"));
    m_products->SetColumnWidth(0, 270);
    m_products->InsertColumn(1, wxT("Version"));
    m_products->SetColumnWidth(1, 110);
    m_products->InsertColumn(2, wxT("Installed version"));
    m_products->SetColumnWidth(2, 110);
    m_products->InsertColumn(3, wxT("State"));
    m_products->SetColumnWidth(3, 135);

    // Set theme for list on MSW for a more native look
#ifdef __WXMSW__
    ::SetWindowTheme((HWND)m_products->GetHWND(), L"Explorer", NULL);
#endif

    // Add application to list
    if (m_progressFile->HasApplication()) {
        auto& application = m_progressFile->GetApplicationWritable();
        addApplication(application);
    }

    // Add components to list
    if (m_progressFile->HaveComponents()) {
        auto& components = m_progressFile->GetComponentsWritable();
        std::for_each(components.begin(), components.end(), [&](Product& component) {
            addComponent(component);
        });
    }
}

void FoundUpdatesDialog::OnClose(wxCloseEvent& event) {
    if (!event.CanVeto()) {
        EndModal(0);
        return;
    }

    if (m_isReadyToInstall) {
        wxRichMessageDialog dialog(
            this,
            _("Updates will not be installed\n\nYou'll be asked again next time unless you delete them."),
            _("Update"),
            wxOK | wxCANCEL | wxCENTER);
        //dialog.ShowDetailedText(_("Delaying installation isn't recommended"))
        dialog.ShowCheckBox(_("Delete updates"));
        if (dialog.ShowModal() == wxID_OK) {
            if (dialog.IsCheckBoxChecked()) {
                m_shouldDeleteUpdates = true;
            }
        }
        else {
            event.Veto();
            return;
        }
    }

    EndModal(0);
}

void FoundUpdatesDialog::deleteUpdates() {
    wxConfig config(m_appName, m_vendorName);
    config.DeleteEntry("Update/ProgressFile");
    m_progressFile->CleanupFiles();
}

bool FoundUpdatesDialog::IsRestartNeeded() const { return m_isRestartRequired; }
bool FoundUpdatesDialog::IsElevationNeeded() const { return m_isElevationNeeded; }
bool FoundUpdatesDialog::IsInstallComplete() const { return m_isInstallComplete; }

void FoundUpdatesDialog::beginDownload() {
    m_verifyHasFailed = false;
    m_downloadHasFailed = false;
    setWorkingState(true);

    Freeze();
    m_downloadProgressPanel->Show();
    Layout();
    Refresh();
    Update();
    Thaw();

    auto cancelCallback = [&]() -> bool {
        return m_shouldStopUpdating;
    };

    DownloadUpdatesThread::BeginDownload(this, *m_progressFile, cancelCallback);
}

void FoundUpdatesDialog::setWorkingState(bool isWorking) {
    m_isWorking = isWorking;
    m_dontUpdate->Enable(isWorking);
    //m_dontInstall->Enable(isWorking);
    if (isWorking) {
        // Can't close window now
        SetWindowStyle(GetWindowStyle() & ~wxCLOSE_BOX);
    }
    else {
        // Can close window now
        SetWindowStyle(GetWindowStyle() | wxCLOSE_BOX);
    }
}

void FoundUpdatesDialog::OnDownloadThreadExit(wxCommandEvent& event)
{
    if (m_downloadHasFailed) {
        wxRichMessageDialog dialog(
            this,
            _("Download failed to complete\n\nPlease make sure that you are connected to the internet, or try again later.\nIf you don't wish to update now, you can resume later unless you delete the updates."),
            _("Download failed"),
            wxOK | wxCANCEL | wxCENTER);
        //dialog.ShowDetailedText(_("Delaying installation isn't recommended"))
        dialog.SetOKCancelLabels(_("Retry"), _("Don't update"));
        dialog.ShowCheckBox(_("Delete updates"));
        if (dialog.ShowModal() != wxID_OK) {
            if (dialog.IsCheckBoxChecked()) {
                m_shouldDeleteUpdates = true;
            }

            Close(true);
            return;
        }

        using namespace aufw::progress;

        if (m_progressFile->HasApplication()) {
            auto& application = m_progressFile->GetApplicationWritable();
            if (application.State == State::DownloadFailed) {
                application.State = State::DownloadPending;
            }
        }

        if (m_progressFile->HaveComponents()) {
            auto& components = m_progressFile->GetComponentsWritable();
            std::for_each(components.begin(), components.end(), [&](Product& component) {
                using namespace aufw::progress;
                if (component.State == State::DownloadFailed) {
                    component.State = State::DownloadPending;
                }
            });
        }

        m_shouldStopUpdating = false;
        beginDownload();
    }
    else if (m_shouldStopUpdating) {
        m_shouldStopUpdating = false;
        setWorkingState(false);
    }
}

void FoundUpdatesDialog::stopUpdating()
{
    if (m_isWorking) {
        m_shouldStopUpdating = true;
    }
}

} } // namespace
