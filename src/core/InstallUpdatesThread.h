#ifndef __InstallUpdatesThread__
#define __InstallUpdatesThread__

#include "CancelCallback.h"
#include <wx/event.h>
#include <wx/thread.h>
#include <memory>
#include <vector>
#include <string>

namespace aufw { namespace progress { class ProgressReaderWriter; struct Product; } }
namespace aufw { namespace job { class Job; } }
namespace aufw { namespace package { class FilePackage; } }

namespace aufw {

class InstallUpdatesThread : public wxThread {
public:
    //
    // Events
    //
    static const wxEventType StateChangedEvent;
    static const wxEventType InstallCompleteEvent;
    static const wxEventType InstallFailedEvent;
    static const wxEventType ServerErrorEvent;
    static const wxEventType ThreadExceptionEvent;
    static const wxEventType ThreadExitEvent;

    InstallUpdatesThread(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback);
    virtual void* Entry();
    virtual void OnExit();
    static void BeginInstall(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback);
    aufw::job::Job* GetJob(aufw::progress::Product& product);

private:
    void install(aufw::progress::Product& product);
    void installNowInternal();
    bool testCanceled();
    aufw::progress::ProgressReaderWriter& m_progressFile;
    wxEvtHandler* m_parent;
    std::unique_ptr<aufw::job::Job> m_job;
    std::unique_ptr<aufw::package::FilePackage> m_package;
    std::vector<std::string> m_fileList;
    CancelCallback_t m_cancelCallback;
    bool m_wasCanceled;
};

} // namespace
#endif // guard
