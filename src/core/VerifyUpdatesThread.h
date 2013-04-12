#ifndef __VerifyUpdatesThread__
#define __VerifyUpdatesThread__

#include "CancelCallback.h"
#include <wx/event.h>
#include <wx/thread.h>

namespace aufw { namespace progress { class ProgressReaderWriter; struct Product; } }

namespace aufw {

class VerifyUpdatesThread : public wxThread {
public:
    //
    // Events
    //
    static const wxEventType StateChangedEvent;
    static const wxEventType VerifyCompleteEvent;
    static const wxEventType VerifyFailedEvent;
    static const wxEventType UpdateErrorEvent;
    static const wxEventType ServerErrorEvent;
    static const wxEventType ThreadExceptionEvent;
    static const wxEventType ThreadExitEvent;

    VerifyUpdatesThread(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback);
    virtual void* Entry();
    virtual void OnExit();
    static void BeginVerify(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback);

private:
    void verify(aufw::progress::Product& product);
    void verifyNowInternal();
    bool testCanceled();
    aufw::progress::ProgressReaderWriter& m_progressFile;
    wxEvtHandler* m_parent;
    CancelCallback_t m_cancelCallback;
    bool m_wasCanceled;
};

} // namespace
#endif // guard
