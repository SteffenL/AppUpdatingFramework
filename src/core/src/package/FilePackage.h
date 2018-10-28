#ifndef __aufw_package_FilePackage__
#define __aufw_package_FilePackage__

#include "Package.h"
#include <string>

namespace aufw { namespace package {

class FilePackage : public Package {
public:
    FilePackage(const std::string& path);
    virtual SourceBase& GetSource();
    virtual ~FilePackage();

protected:
    std::string m_path;
};

} } // namespace
#endif // guard
