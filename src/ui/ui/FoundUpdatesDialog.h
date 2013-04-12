#ifndef __FoundUpdatesDialog__
#define __FoundUpdatesDialog__

#include "gen_aufw_ui_main.h"
#include <wx/event.h>
#include <memory>
#include <map>

class wxWebView;

namespace aufw { namespace progress { struct Product; class ProgressReaderWriter; } }
namespace aufw { namespace job { struct StepProgressArg; } }

namespace aufw { namespace ui {

class RestartRequiredEvent : public wxCommandEvent {
public:
    RestartRequiredEvent();
    wxEvent* Clone() const;
    bool IsElevationNeeded() const;
    void SetElevationNeeded(bool v);
    bool CanVeto() const;
    void SetCanVeto(bool v);
    void Veto(bool v = true);
    bool GetVeto() const;
    bool GetContinueInstallUpdates() const;
    void SetContinueInstallUpdates(bool v);

private:
    bool m_elevationNeeded;
    bool m_veto;
    bool m_canVeto;
    bool m_continueInstallUpdates;
};

wxDECLARE_EVENT(EVT_RESTART_REQUIRED, RestartRequiredEvent);

class FoundUpdatesDialog : public FoundUpdatesDialogBase {
protected:
    //
    // Event handlers
    //
    virtual void OnStateChanged(wxCommandEvent& event);
    virtual void OnDownloadsComplete(wxCommandEvent& event);
    virtual void OnVerifyComplete(wxCommandEvent& event);
    virtual void OnVerifyFailed(wxCommandEvent& event);
    virtual void OnVerifyThreadExit(wxCommandEvent& event);
    virtual void OnInstallComplete(wxCommandEvent& event);
    virtual void OnInstallThreadException(wxCommandEvent& event);
    virtual void OnDownloadProgress(wxCommandEvent& event);
    virtual void OnDownloadFailed(wxCommandEvent& event);
    virtual void OnDownloadThreadExit(wxCommandEvent& event);
    virtual void OnDownloadThreadException(wxCommandEvent& event);
    virtual void OnUpdateError(wxCommandEvent& event);
    virtual void OnServerError(wxCommandEvent& event);
    virtual void OnThreadException(wxCommandEvent& event);
    //virtual void OnJobStepProgress(aufw::job::StepProgressArg& arg);

    //
    // Inherited event handlers
    //
    virtual void OnInitDialog(wxInitDialogEvent& event);
    virtual void updateNowOnButtonClick(wxCommandEvent& event);
    virtual void skipUpdateOnButtonClick(wxCommandEvent& event);
    virtual void dontUpdateOnButtonClick(wxCommandEvent& event);
    virtual void productsOnListItemSelected(wxListEvent& event);
    virtual void moreInfoOnMenuSelection(wxCommandEvent& event);
    virtual void moreInfoOnUpdateUI(wxUpdateUIEvent& event);
    virtual void manualDownloadOnMenuSelection(wxCommandEvent& event);
    virtual void manualDownloadOnUpdateUI(wxUpdateUIEvent& event);
    virtual void installNowOnButtonClick(wxCommandEvent& event);
    virtual void dontInstallOnButtonClick(wxCommandEvent& event);
    virtual void OnClose(wxCloseEvent& event);

public:
    FoundUpdatesDialog(wxWindow* parent, aufw::progress::ProgressReaderWriter* progressFile,
        const wxString& appName, const wxString& vendorName, bool autoStartInstall);
    virtual ~FoundUpdatesDialog();

    bool IsRestartNeeded() const;
    bool IsElevationNeeded() const;
    bool IsInstallComplete() const;

private:
    DECLARE_EVENT_TABLE()

    std::unique_ptr<aufw::progress::ProgressReaderWriter> m_progressFile;
    wxString m_appName;
    wxString m_vendorName;
    wxWebView* m_releaseNotes;
    std::map<aufw::progress::Product*, long> m_productListIndexMap;
    bool m_isReadyToInstall;
    bool m_isInstalling;
    bool m_autoStartInstall;

    bool m_isRestartRequired;
    bool m_isElevationNeeded;
    bool m_isInstallComplete;
    bool m_isWorking;

    bool m_verifyHasFailed;
    bool m_downloadHasFailed;
    bool m_shouldStopUpdating;
    bool m_shouldDeleteUpdates;

    void addApplication(aufw::progress::Product& product);
    void addComponent(aufw::progress::Product& product);
    void hideAllPanels();
    void setupPanels();
    void setupProductList();
    void deleteUpdates();
    void beginDownload();
    void setWorkingState(bool isWorking);
    void stopUpdating();
};

} } // namespace
#endif // guard
