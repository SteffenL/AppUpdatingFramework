#ifndef __aufw_package_sinks__
#define __aufw_package_sinks__

#include "sources.h"

#include <string>

namespace aufw { namespace package {

class SinkBase {
public:
    virtual void Consume(SourceBase& source) = 0;
    virtual ~SinkBase();
};

class DirectorySink : public SinkBase {
public:
    DirectorySink(const std::string& path);
    virtual void Consume(SourceBase& source);
    std::vector<std::string>& GetCompleteFiles();

private:
    std::string m_path;
    std::vector<std::string> m_completeFiles;
};

} } // namespace
#endif // guard
