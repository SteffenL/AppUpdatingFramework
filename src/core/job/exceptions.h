#ifndef __aufw_package_job_exceptions__
#define __aufw_package_job_exceptions__

#include <string>
#include <exception>

namespace aufw { namespace job {

class JobException : public std::runtime_error {
public:
    JobException(const std::string& message) : runtime_error(message) {}
};

} } // namespace
#endif // guard
