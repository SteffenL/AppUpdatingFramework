#ifndef __DownloadUpdatesThread__
#define __DownloadUpdatesThread__

#include "CancelCallback.h"
#include <wx/event.h>
#include <wx/thread.h>

namespace aufw { namespace progress { class ProgressReaderWriter; struct Product; } }

namespace aufw {

class DownloadUpdatesThread : public wxThread {
public:
    //
    // Events
    //
    static const wxEventType StateChangedEvent;
    static const wxEventType DownloadsCompleteEvent;
    static const wxEventType DownloadProgressEvent;
    static const wxEventType DownloadFailedEvent;
    static const wxEventType UpdateErrorEvent;
    static const wxEventType ServerErrorEvent;
    static const wxEventType ThreadExceptionEvent;
    static const wxEventType ThreadExitEvent;

    DownloadUpdatesThread(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback);
    virtual void* Entry();
    virtual void OnExit();
    static void BeginDownload(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback);
    bool TestCanceled();

private:
    void download(aufw::progress::Product& product);
    void downloadNowInternal();
    aufw::progress::ProgressReaderWriter& m_progressFile;
    wxEvtHandler* m_parent;
    CancelCallback_t m_cancelCallback;
    bool m_wasCanceled;
    
};

} // namespace
#endif // guard