#include "StartProgramStep.h"
#include "../exceptions.h"

#include <boost/filesystem.hpp>
#include <Poco/Process.h>

namespace aufw { namespace job {

//
// Implementation
//

class StartProgramStep::Impl {
public:
    Impl(const std::string& exePath);
    Impl(const std::string& exePath, const std::vector<std::string>& arguments);
    void Execute();
    void Rollback();

private:
    std::string m_exePath;
    std::vector<std::string> m_arguments;
    // I don't like making ProcessHandle a (smart) pointer, but its default constructor is private.
    std::unique_ptr<Poco::ProcessHandle> m_processHandle;
};

StartProgramStep::Impl::Impl(const std::string& exePath) : m_exePath(exePath) {}
StartProgramStep::Impl::Impl(const std::string& exePath, const std::vector<std::string>& arguments)
    : m_exePath(exePath), m_arguments(arguments) {}

void StartProgramStep::Impl::Execute() {
    auto& handle = Poco::Process::launch(m_exePath, m_arguments);
    m_processHandle = std::unique_ptr<Poco::ProcessHandle>(new Poco::ProcessHandle(handle));
}

void StartProgramStep::Impl::Rollback() {
    Poco::Process::kill(*m_processHandle);
    m_processHandle->wait();
}

//
// Public
//

StartProgramStep::StartProgramStep(){}
StartProgramStep::StartProgramStep(const std::string& exePath) : m_impl(new Impl(exePath)) {}
StartProgramStep::StartProgramStep(const std::string& exePath, const std::vector<std::string>& arguments) : m_impl(new Impl(exePath, arguments)) {}
StartProgramStep::~StartProgramStep() {}

void StartProgramStep::Execute() {
    m_impl->Execute();
}

void StartProgramStep::Rollback() {
    m_impl->Rollback();
}

std::string StartProgramStep::GetTypeName_() {
    return Step::getTypeNameInternal(typeid(StartProgramStep));
}

} } // namespace
