#ifndef __aufw_package_Job__
#define __aufw_package_Job__

#include "Step.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <stdexcept>

namespace aufw { namespace job {

class Job;

struct JobFailureArg {
    bool ShouldRetry;
    std::exception* Exception;
    JobFailureArg() : ShouldRetry(false), Exception(nullptr) {}
};

struct StepProgressArg {
    const Job* Job;
    const Step* Step;
};

class Job {
public:

    Job();
    bool Execute();
    void Rollback();
    void AddStep(Step* step);
    virtual void Serialize(boost::archive::xml_iarchive& ar);
    virtual void Serialize(boost::archive::xml_oarchive& ar);
    std::vector<std::unique_ptr<Step>>& GetSteps();

protected:
    virtual void onExecutionFailed(JobFailureArg& arg);
    virtual void onRollbackFailed(JobFailureArg& arg);
    virtual void onExecutionSuccess();
    virtual void onRollbackSuccess();
    virtual void onStepProgress(StepProgressArg& arg);

public:
    std::function<void (JobFailureArg&)> OnExecutionFailed;
    std::function<void (JobFailureArg&)> OnRollbackFailed;
    std::function<void ()> OnExecutionSuccess;
    std::function<void ()> OnRollbackSuccess;
    std::function<void (StepProgressArg&)> OnStepProgress;

protected:
    std::vector<std::unique_ptr<Step>> m_steps;
    int m_stepIndex;
};

} } // namespace
#endif // guard
