#ifndef __aufw_Process_msw__
#define __aufw_Process_msw__

#include "ProcessBase.h"
#include <string>
#include <vector>

namespace aufw {

class Process_msw : public ProcessBase {
public:
    static unsigned long LaunchElevated(const std::string& path, const std::string& params = std::string(), void* parentWindow = nullptr);
    static unsigned long LaunchNonElevated(const std::string& path, const std::string& params = std::string());
    static unsigned long Launch(const std::string& path, const std::string& params = std::string());
    static unsigned long GetCurrentPid();
    static void FindByName(const std::string& name, std::vector<unsigned long>& pidList);
    static bool Terminate(unsigned long pid);

private:
    static void* getLoggedOnUserImpersonationToken();
    static void setTokenPrivilege(void* token, const std::wstring& name, bool enable);
    static void setProcessPrivilege(void* process, const std::wstring& name, bool enable);
    static void setProcessPrivilege(const std::wstring& name, bool enable);
};

} // namespace
#endif // guard
