#ifndef __aufw_package_Package__
#define __aufw_package_Package__

#include "sources.h"
#include "sinks.h"

#include <string>
#include <iostream>
#include <vector>
#include <memory>

namespace aufw { namespace package {

class Package {
public:
    virtual SourceBase& GetSource() = 0;
    virtual ~Package();

protected:
    bool hasSource() const;
    void setSource(SourceBase* source);

protected:
    std::unique_ptr<SourceBase> m_source;
};

} } // namespace
#endif // guard
