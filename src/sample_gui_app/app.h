#ifndef __MyApp__
#define __MyApp__

#include <wx/wxprec.h>

#define APP_NAME "SampleGuiApp"
#define VENDOR_NAME "CompanyName"
#define APP_RELEASE_CHANNEL "main"

struct StartupMode {
    enum type {
        Main,
        InstallUpdates,
        ContinueInstallUpdates
    };
};

class MyApp : public wxApp {
public:
    MyApp();
    bool OnInit();
    int OnExit();
    void OnInitCmdLine(wxCmdLineParser& parser);
    bool OnCmdLineParsed(wxCmdLineParser& parser);

private:
    bool mainStartup();
    bool updateStartup(const wxString& progressFilePath, bool autoStartInstall);
    static void restartApp(bool continueInstallUpdates, bool elevate, bool& veto, void* parentWindow);
    static void restartApp(bool elevate, void* parentWindow);

    int m_exitCode;
    StartupMode::type m_startupMode;
};

DECLARE_APP(MyApp)

#endif // guard
