#include "ProcessBase.h"
#include <stdexcept>

namespace aufw {

unsigned long ProcessBase::LaunchElevated(const std::string& path, const std::string& params, void* parentWindow) {
    throw std::logic_error("Not implemented");
}

unsigned long ProcessBase::LaunchNonElevated(const std::string& path, const std::string& params) {
    throw std::logic_error("Not implemented");
}

unsigned long ProcessBase::Launch(const std::string& path, const std::string& params) {
    throw std::logic_error("Not implemented");
}

unsigned long ProcessBase::GetCurrentPid() {
    throw std::logic_error("Not implemented");
}

void ProcessBase::FindByName(const std::string& name, std::vector<unsigned long>& pidList) {
    throw std::logic_error("Not implemented");
}

bool ProcessBase::Terminate(unsigned long pid) {
    throw std::logic_error("Not implemented");
}

} // namespace
