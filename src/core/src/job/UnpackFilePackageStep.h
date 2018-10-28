#ifndef __aufw_package_UnpackFilePackageStep__
#define __aufw_package_UnpackFilePackageStep__

#include <aufw/core/job/Step.h>
#include "../package/FilePackage.h"

#include <string>
#include <vector>

namespace boost { namespace archive { class xml_iarchive; class xml_oarchive; } }

namespace aufw { namespace job {

class UnpackFilePackageStep : public Step {
public:
    UnpackFilePackageStep();
    UnpackFilePackageStep(package::FilePackage* package, const std::string& targetDir, const std::string& backupDir = "");
    void Init(package::FilePackage* package, const std::string& targetDir, const std::string& backupDir = "");
    virtual void Execute();
    virtual void Rollback();
    virtual void Serialize(boost::archive::xml_iarchive& ar);
    virtual void Serialize(boost::archive::xml_oarchive& ar);
    static std::string GetTypeName_();

protected:
    package::FilePackage* m_package;
    std::string m_targetDir;
    std::string m_backupDir;
    std::vector<std::string> m_fileList;
    std::vector<std::string> m_completeFiles;
};

} } // namespace
#endif // guard
