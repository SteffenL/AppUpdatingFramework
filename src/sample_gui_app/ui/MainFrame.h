#ifndef __MainFrame__
#define __MainFrame__

#include "gen_main.h"

namespace aufw { namespace ui { class RestartRequiredEvent; } }

namespace ui {

class MainFrame : public MainFrameBase {
protected:
    //
    // Event handlers
    //
    virtual void OnUpdateFound(wxCommandEvent& event);
    virtual void OnUpdateNotFound(wxCommandEvent& event);
    virtual void OnUpdateError(wxCommandEvent& event);
    virtual void OnServerError(wxCommandEvent& event);
    virtual void OnThreadException(wxCommandEvent& event);
    virtual void OnRestartRequired(aufw::ui::RestartRequiredEvent& event);

    //
    // Inherited event handlers
    //
    void checkForUpdatesOnButtonClick(wxCommandEvent& event);

public:
    MainFrame(wxWindow* parent);

private:
    DECLARE_EVENT_TABLE()

    void restartApp(bool continueInstallUpdates, bool elevate, bool& veto);
    void restartApp(bool elevate);

    bool m_shouldBlockCheckForUpdates;
};

} // namespace
#endif // guard
