#ifndef __aufw_ProcessBase__
#define __aufw_ProcessBase__

#include <string>
#include <vector>

namespace aufw {

class ProcessBase {
public:
    static unsigned long LaunchElevated(const std::string& path, const std::string& params, void* parentWindow = nullptr);
    static unsigned long LaunchNonElevated(const std::string& path, const std::string& params);
    static unsigned long Launch(const std::string& path, const std::string& params);
    static unsigned long GetCurrentPid();
    static void FindByName(const std::string& name, std::vector<unsigned long>& pidList);
    static bool Terminate(unsigned long pid);
};

} // namespace
#endif // guard
