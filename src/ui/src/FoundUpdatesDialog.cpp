#include <aufw/ui/FoundUpdatesDialog.h>
#include "Texts.h"
#include <aufw/core/FindUpdatesThread.h>
#include <aufw/core/DownloadUpdatesThread.h>
#include <aufw/core/VerifyUpdatesThread.h>
#include <aufw/core/InstallUpdatesThread.h>

#include <aufw/core/progress/ProgressReaderWriter.h>
#include <aufw/core/progress/Product.h>
#include <aufw/core/progress/State.h>
#include <aufw/core/Process.h>
#include <aufw/core/Elevation.h>

#include <wx/webview.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>
#include <wx/config.h>
#include <wx/log.h>

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
    // Progress
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::DownloadProgressEvent, FoundUpdatesDialog::OnDownloadProgress)

    // Failed
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::DownloadFailedEvent, FoundUpdatesDialog::OnDownloadFailed)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::VerifyFailedEvent, FoundUpdatesDialog::OnVerifyFailed)
    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::InstallFailedEvent, FoundUpdatesDialog::OnInstallFailed)

    // Complete
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::DownloadsCompleteEvent, FoundUpdatesDialog::OnDownloadsComplete)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::VerifyCompleteEvent, FoundUpdatesDialog::OnVerifyComplete)
    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::InstallCompleteEvent, FoundUpdatesDialog::OnInstallComplete)

    // State changed
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::StateChangedEvent, FoundUpdatesDialog::OnStateChanged)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::StateChangedEvent, FoundUpdatesDialog::OnStateChanged)
    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::StateChangedEvent, FoundUpdatesDialog::OnStateChanged)

    // Thread exit
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::ThreadExitEvent, FoundUpdatesDialog::OnDownloadThreadExit)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::ThreadExitEvent, FoundUpdatesDialog::OnVerifyThreadExit)
    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::ThreadExitEvent, FoundUpdatesDialog::OnInstallThreadExit)

    // Thread exception
    EVT_COMMAND(wxID_ANY, DownloadUpdatesThread::ThreadExceptionEvent, FoundUpdatesDialog::OnDownloadThreadException)
    EVT_COMMAND(wxID_ANY, VerifyUpdatesThread::ThreadExceptionEvent, FoundUpdatesDialog::OnVerifyThreadException)
    EVT_COMMAND(wxID_ANY, InstallUpdatesThread::ThreadExceptionEvent, FoundUpdatesDialog::OnInstallThreadException)
END_EVENT_TABLE()

FoundUpdatesDialog::FoundUpdatesDialog(wxWindow* parent, aufw::progress::ProgressReaderWriter* progressFile,
    const wxString& appName, const wxString& vendorName, bool autoStartInstall)
    : FoundUpdatesDialogBase(parent),
    m_progressFile(progressFile),
    m_appName(appName),
    m_vendorName(vendorName),
    m_autoStartInstall(autoStartInstall),
    m_isRestartRequired(false),
    m_shouldDeleteProgress(false),
    m_currentWorkType(WorkType::None),
    m_isElevationNeeded(false),
    m_wasCanceled(false)
{
    resetWorkState();
}

// Note: Do not cleanup the update progress files since they are still in use
// Wait until the app has restarted

FoundUpdatesDialog::~FoundUpdatesDialog() {
    // Cleanup if needed
    if (m_shouldDeleteProgress)
    {
        deleteProgress();
    }
}

void FoundUpdatesDialog::OnInitDialog(wxInitDialogEvent& event) {
    //m_progressFile->Load();
    m_appRestartText->SetLabel(wxString::Format(_("%s will restart automatically after installation."), m_appName));
    m_currentWorkType = getRealCurrentWorkType();

#ifdef __WXMSW__
    // Set shield icon on button
    if (!Elevation::IsUserAdmin()) {
        Button_SetElevationRequiredState(static_cast<HWND>(m_updateNow->GetHandle()), true);
        Button_SetElevationRequiredState(static_cast<HWND>(m_installNow->GetHandle()), true);
    }
#else
#error Not implemented
#endif

    // Add web view
    m_releaseNotes = wxWebView::New(m_releaseNotesContainer, wxID_ANY);
    m_releaseNotesContainer->GetSizer()->Add(m_releaseNotes, wxSizerFlags(1).Expand());
    m_releaseNotesContainer->Layout();

    setupUi();
    setupProductList();

    // Select the first item in the list to load the URL
    if (m_products->GetItemCount() > 0) {
        m_products->SetFocus();
        m_products->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }

    if (m_autoStartInstall) {
        beginNextWork();
    }
}

void FoundUpdatesDialog::updateNowOnButtonClick(wxCommandEvent& event) {
    saveProgress();
    beginNextWork();
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
        stopWork();
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
    m_products->SetItem(itemIndex, 3, wxString::FromUTF8(Texts::GetStateText(product.State).c_str()));
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
    m_products->SetItem(itemIndex, 3, wxString::FromUTF8(Texts::GetStateText(product.State).c_str()));
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


void FoundUpdatesDialog::installNowOnButtonClick(wxCommandEvent& event) {
    beginWork(WorkType::Install);
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

void FoundUpdatesDialog::setupUi() {
    using namespace aufw;

    Freeze();
    hideAllPanels();

    switch (m_currentWorkType) {
    case WorkType::None:
        setupPreWorkUi();
        break;
    case WorkType::Download:
        setupDownloadUi();
        break;
    case WorkType::Verify:
        setupVerifyUi();
        break;
    case WorkType::Install:
        setupInstallUi();
        break;
    case WorkType::PostInstall:
        setupPostInstallUi();
        break;
    default:
        assert(false);
        throw std::logic_error("Invalid work type");
    }

    Layout();
    Refresh();
    Update();
    Thaw();
}

void FoundUpdatesDialog::setupProductList() {
    using namespace aufw::progress;

    // Insert columns
    m_products->InsertColumn(0, _("Product"));
    m_products->SetColumnWidth(0, 270);
    m_products->InsertColumn(1, _("Version"));
    m_products->SetColumnWidth(1, 110);
    m_products->InsertColumn(2, _("Installed version"));
    m_products->SetColumnWidth(2, 110);
    m_products->InsertColumn(3, _("State"));
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
    event.Skip();

    if (!event.CanVeto()) {
        EndModal(0);
        return;
    }

    if (m_progressFile->HasProgress()) {
        wxRichMessageDialog dialog(
            this,
            _("Updates will not be installed\n\nYou'll be asked again next time unless you delete them."),
            _("Update"),
            wxOK | wxCANCEL | wxCENTER);
        //dialog.ShowDetailedText(_("Delaying installation isn't recommended"))
        dialog.ShowCheckBox(_("Delete updates"));
        if (dialog.ShowModal() == wxID_OK) {
            m_wasCanceled = true;
            if (dialog.IsCheckBoxChecked()) {
                m_shouldDeleteProgress = true;
            }
        }
        else {
            event.Veto();
            return;
        }
    }

    EndModal(0);
}

void FoundUpdatesDialog::deleteProgress() {
    wxConfig config(m_appName, m_vendorName);
    config.DeleteEntry("Update/ProgressFile");
    m_progressFile->CleanupFiles();
}

void FoundUpdatesDialog::saveProgress() {
    m_progressFile->Save();
    // Save progress file path
    wxConfig config(m_appName, m_vendorName);
    config.Write("Update/ProgressFile", wxString(m_progressFile->GetFilePath()));
}

bool FoundUpdatesDialog::IsRestartNeeded() const { return m_isRestartRequired; }
bool FoundUpdatesDialog::IsElevationNeeded() const { return m_isElevationNeeded; }
bool FoundUpdatesDialog::IsInstallComplete() const { return (m_currentWorkType == WorkType::Install) && m_workHasCompleted; }

void FoundUpdatesDialog::beginDownload() {
    DownloadUpdatesThread::BeginDownload(this, *m_progressFile, std::bind(&FoundUpdatesDialog::stopWorkCallback, this));
}

void FoundUpdatesDialog::beginVerify() {
    VerifyUpdatesThread::BeginVerify(this, *m_progressFile, std::bind(&FoundUpdatesDialog::stopWorkCallback, this));
}

void FoundUpdatesDialog::beginInstall() {
    InstallUpdatesThread::BeginInstall(this, *m_progressFile, std::bind(&FoundUpdatesDialog::stopWorkCallback, this));
}

void FoundUpdatesDialog::beginPostInstall() {
    // Notify that restart is required
    wxQueueEvent(m_parent, RestartRequiredEvent().Clone());

    m_shouldDeleteProgress = true;
    Close(true);
}

void FoundUpdatesDialog::stopWork() {
    m_shouldStopWork = true;
}

//////////////////////////////////////////////////////////////////////////
// Event handlers
//////////////////////////////////////////////////////////////////////////

//
// Progress
//

void FoundUpdatesDialog::OnDownloadProgress(wxCommandEvent& event) {
    m_progressGauge->SetValue(event.GetInt());
}

//
// Failed
//

void FoundUpdatesDialog::OnDownloadFailed(wxCommandEvent& event) {
    m_workHasFailed = true;
}

void FoundUpdatesDialog::OnVerifyFailed(wxCommandEvent& event) {
    m_workHasFailed = true;
}

void FoundUpdatesDialog::OnInstallFailed(wxCommandEvent& event) {
    m_workHasFailed = true;
}

//
// Complete
//

void FoundUpdatesDialog::OnDownloadsComplete(wxCommandEvent& event) {
    m_workHasCompleted = true;
}

void FoundUpdatesDialog::OnVerifyComplete(wxCommandEvent& event) {
    m_workHasCompleted = true;
}

void FoundUpdatesDialog::OnInstallComplete(wxCommandEvent& event) {
    m_workHasCompleted = true;
    m_isRestartRequired = true;
}

//
// State changed
//

void FoundUpdatesDialog::OnStateChanged(wxCommandEvent& event) {
    using namespace aufw::progress;
    // Update UI
    auto& product = *static_cast<Product*>(event.GetClientData());
    auto itemIndex = m_productListIndexMap[&product];
    m_products->SetItem(itemIndex, 3, wxString::FromUTF8(Texts::GetStateText(product.State).c_str()));
}

//
// Thread exit
//

void FoundUpdatesDialog::OnDownloadThreadExit(wxCommandEvent& event) {
    if (workHasCompleted()) {
        beginNextWork();
    }
    else if (workHasFailed()) {
        // Display error
        // Offer to retry

        wxRichMessageDialog dialog(
            this,
            wxString::Format(_("%s\n\nYou may continue later; however, to make sure that you download the latest updates, "
                "waiting too long before you continue is not recommended."), _("Download failed")),
            _("Download failed"),
            wxOK | wxCANCEL | wxCENTER);
        dialog.ShowCheckBox(_("Continue later"), true);
        dialog.SetOKCancelLabels(_("Retry"), _("Cancel"));
        if (dialog.ShowModal() == wxID_CANCEL) {
            if (!dialog.IsCheckBoxChecked()) {
                m_shouldDeleteProgress = true;
            }

            Close(true);
        }
        else {
            // Retry
            beginWork(WorkType::Download);
        }
    }
    else if (workWasStopped()) {
        // Display message
        wxRichMessageDialog dialog(
            this,
            wxString::Format(_("%s\n\nYou may continue later; however, to make sure that you install the latest updates, "
                "waiting too long before you continue is not recommended."), _("Download was stopped")),
            _("Download was stopped"),
            wxOK | wxCENTER);
        dialog.ShowCheckBox(_("Continue later"), true);
        dialog.ShowModal();
        if (!dialog.IsCheckBoxChecked()) {
            m_shouldDeleteProgress = true;
        }

        Close(true);
    }
}

void FoundUpdatesDialog::OnVerifyThreadExit(wxCommandEvent& event) {
    if (workHasCompleted()) {
        beginNextWork();
    }
    else if (workHasFailed()) {
        // Display error
        // Offer to retry

        wxRichMessageDialog dialog(
            this,
            wxString::Format(_("%s\n\nThis may, for example, happen if the internet connection is unstable, or because of hardware faults. "
                "For security reasons, non-verified files will not be installed."), _("A downloaded file is damaged")),
            _("Verification failed"),
            wxOK | wxCANCEL | wxCENTER);
        dialog.ShowCheckBox(_("Continue later"), true);
        dialog.SetOKCancelLabels(_("Retry"), _("Cancel"));
        if (dialog.ShowModal() == wxID_CANCEL) {
            if (!dialog.IsCheckBoxChecked()) {
                m_shouldDeleteProgress = true;
            }

            Close(true);
        }
        else {
            // Retry
            beginWork(WorkType::Download);
        }
    }
    else if (workWasStopped()) {
        // Display message
        wxRichMessageDialog dialog(
            this,
            wxString::Format(_("%s\n\nYou may continue later; however, to make sure that you install the latest updates, waiting too long before you continue is not recommended."), _("Verification was stopped")),
            _("Verification was stopped"),
            wxOK | wxCENTER);
        dialog.ShowCheckBox(_("Continue later"), true);
        dialog.ShowModal();
        if (!dialog.IsCheckBoxChecked()) {
            m_shouldDeleteProgress = true;
        }

        Close(true);
    }
}

void FoundUpdatesDialog::OnInstallThreadExit(wxCommandEvent& event) {
    if (workHasCompleted()) {
        beginNextWork();
    }
    else if (workHasFailed()) {
        // Maybe elevation is needed
        if (!Elevation::IsUserAdmin()) {
            // Notify that restart is required (elevated)
            RestartRequiredEvent e;
            e.SetElevationNeeded(true);
            e.SetContinueInstallUpdates(true);
            bool canClose = true;
            if (GetParent()->GetEventHandler()->ProcessEvent(e)) {
                if (e.GetVeto()) {
                    canClose = false;
                }
            }

            if (canClose) {
                m_isRestartRequired = true;
                m_isElevationNeeded = true;
                Close(true);
                return;
            }

            resetWorkState();
            setupUi();
        }
        else {
            // Display error
            // Offer to retry
        }
    }
    else if (workWasStopped()) {
        // Display message
    }
}

//
// Thread exception
//

void FoundUpdatesDialog::OnDownloadThreadException(wxCommandEvent& event) {
    OnThreadException(event);
}

void FoundUpdatesDialog::OnVerifyThreadException(wxCommandEvent& event) {
    OnThreadException(event);
}

void FoundUpdatesDialog::OnInstallThreadException(wxCommandEvent& event) {
    OnThreadException(event);
}

void FoundUpdatesDialog::OnThreadException(wxCommandEvent& event) {
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
            wxLogError(ex.what());
        }
    }
}

bool FoundUpdatesDialog::workHasFailed() const {
    return m_workHasFailed;
}

bool FoundUpdatesDialog::workHasCompleted() const {
    return m_workHasCompleted;
}

bool FoundUpdatesDialog::workWasStopped() const {
    return m_shouldStopWork;
}

void FoundUpdatesDialog::resetWorkState() {
    m_workHasFailed = false;
    m_workHasCompleted = false;
    m_isWorking = false;
    m_shouldStopWork = false;
    m_isRestartRequired = false;
    m_isElevationNeeded = false;
}

void FoundUpdatesDialog::prepareWorkState() {
    m_workHasFailed = false;
    m_workHasCompleted = false;
    m_isWorking = false;
}

bool FoundUpdatesDialog::stopWorkCallback() {
    return m_shouldStopWork;
}

void FoundUpdatesDialog::beginWork(WorkType::type workType) {
    prepareWorkState();
    m_currentWorkType = workType;
    m_isWorking = true;
    setupUi();

    switch (workType) {
    case WorkType::Download:
        beginDownload();
        break;
    case WorkType::Verify:
        beginVerify();
        break;
    case WorkType::Install:
        beginInstall();
        break;
    case WorkType::PostInstall:
        beginPostInstall();
        break;
    default:
        assert(false);
        throw std::logic_error("Invalid work type");
    }
}

void FoundUpdatesDialog::setupPreWorkUi() {
    m_headerText->SetLabel(_("Updates have been released!"));

    m_headerPanel->Show();
    m_releaseNotesContainer->Show();
    m_productsPanel->Show();
    m_updateButtonsPanel->Show();

    m_updateNow->Enable(!m_isWorking);
}

void FoundUpdatesDialog::setupDownloadUi() {
    if (m_isWorking) {
        m_headerText->SetLabel(_("Updates are being downloaded."));
    }
    else {
        m_headerText->SetLabel(_("Updates are ready for download!"));
    }

    m_headerPanel->Show();
    m_releaseNotesContainer->Show();
    m_productsPanel->Show();
    m_updateButtonsPanel->Show();

    m_progressGauge->SetValue(0);
    m_progressPanel->Show();

    m_updateNow->Enable(!m_isWorking);
}

void FoundUpdatesDialog::setupVerifyUi() {
    if (m_isWorking) {
        m_headerText->SetLabel(_("Updates are being verified."));
    }
    else {
        m_headerText->SetLabel(_("Updates are ready to be verified."));
    }

    m_headerPanel->Show();
    m_releaseNotesContainer->Show();
    m_productsPanel->Show();
    m_updateButtonsPanel->Show();

    m_updateNow->Enable(!m_isWorking);
}

void FoundUpdatesDialog::setupInstallUi() {
    if (m_isWorking) {
        m_headerText->SetLabel(_("Updates are being installed."));
    }
    else {
        m_headerText->SetLabel(_("Updates are ready to be installed."));
    }

    m_headerPanel->Show();
    m_releaseNotesContainer->Show();
    m_productsPanel->Show();
    m_installButtonsPanel->Show();

    m_installNow->Enable(!m_isWorking);
}

void FoundUpdatesDialog::setupPostInstallUi() {}

void FoundUpdatesDialog::beginNextWork() {
    if (m_progressFile->InstallIsComplete()) {
        beginWork(WorkType::PostInstall);
    }
    else if (m_progressFile->IsReadyToInstall()) {
        beginWork(WorkType::Install);
    }
    else if (m_progressFile->IsReadyToVerify()) {
        beginWork(WorkType::Verify);
    }
    else if (m_progressFile->IsReadyToDownload()) {
        beginWork(WorkType::Download);
    }
}

FoundUpdatesDialog::WorkType::type FoundUpdatesDialog::getRealCurrentWorkType() const {
    if (m_progressFile->InstallIsComplete()) {
        return WorkType::PostInstall;
    }
    else if (m_progressFile->IsReadyToInstall()) {
        return WorkType::Install;
    }
    else if (m_progressFile->IsReadyToVerify()) {
        return WorkType::Verify;
    }
    else if (m_progressFile->IsReadyToDownload()) {
        return WorkType::Download;
    }
    else {
        return WorkType::None;
    }
}

bool FoundUpdatesDialog::WasCanceled() const
{
    return m_wasCanceled;
}

} } // namespace
