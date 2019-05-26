#ifndef __UpdateThreadThread__
#define __UpdateThreadThread__

#include "ProductInstallationInfo.h"
#include "SearchMethod.h"
#include <wx/event.h>
#include <wx/thread.h>
#include <functional>
#include <list>
#include <string>

namespace aufw {

class FindUpdatesThread : public wxThread {
public:
    typedef std::function<void(ProductInstallationInfo&, std::list<ProductInstallationInfo>&)> OnCollect_t;

    //
    // Events
    //
    static const wxEventType UpdateFoundEvent;
    static const wxEventType UpdateNotFoundEvent;
    static const wxEventType UpdateErrorEvent;
    static const wxEventType ServerErrorEvent;
    static const wxEventType ThreadExceptionEvent;

    FindUpdatesThread(wxEvtHandler* parent, const std::string& appName, const std::string& vendorName, const std::string& appReleaseChannel, SearchMethod::type searchMethod);
    virtual void* Entry();
    virtual void OnExit();
    std::string GetProgessFilePath() const;
    static void BeginCheck(wxEvtHandler* parent, const std::string& appName, const std::string& vendorName, const std::string& appReleaseChannel, SearchMethod::type searchMethod, OnCollect_t onCollect);

    OnCollect_t OnCollect;

private:
    void checkNowInternal();
    std::string m_progressFilePath;
    std::string m_vendorName;
    std::string m_appName;
	std::string m_appReleaseChannel;
    wxEvtHandler* m_parent;
	SearchMethod::type m_searchMethod;
};

} // namespace
#endif // guard