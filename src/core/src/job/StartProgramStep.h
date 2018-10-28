#ifndef __aufw_package_StartProgramStep__
#define __aufw_package_StartProgramStep__

#include <aufw/core/job/Step.h>

#include <string>
#include <vector>
#include <memory>

namespace aufw { namespace job {

class StartProgramStep : public Step {
public:
    StartProgramStep();
    StartProgramStep(const std::string& exePath);
    StartProgramStep(const std::string& exePath, const std::vector<std::string>& arguments);
    virtual ~StartProgramStep();
    void Execute();
    void Rollback();
    static std::string GetTypeName_();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} } // namespace
#endif // guard
