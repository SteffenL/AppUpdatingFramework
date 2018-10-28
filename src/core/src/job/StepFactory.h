#ifndef __aufw_job_StepFactory__
#define __aufw_job_StepFactory__

#include <string>

namespace aufw { namespace job {

class Step;

class StepFactory {
public:
    static Step* CreateFromName(const std::string& name);
};

} } // namespace
#endif // guard
