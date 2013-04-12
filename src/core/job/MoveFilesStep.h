#ifndef __aufw_package_MovesFileStep__
#define __aufw_package_MovesFileStep__

#include "Step.h"

#include <string>
#include <vector>

namespace boost { namespace archive { class xml_iarchive; class xml_oarchive; } }

namespace aufw { namespace job {

class MoveFilesStep : public Step {
public:
    MoveFilesStep();
    MoveFilesStep(const std::string& baseDir, const std::vector<std::string>& fileList, const std::string& targetDir);
    void Init(const std::string& baseDir, const std::vector<std::string>& fileList, const std::string& targetDir);
    virtual void Execute();
    virtual void Rollback();
    virtual void Serialize(boost::archive::xml_iarchive& ar);
    virtual void Serialize(boost::archive::xml_oarchive& ar);
    static std::string GetTypeName_();

protected:
    std::string m_baseDir;
    std::vector<std::string> m_fileList;
    std::vector<std::string> m_completeFiles;
    std::string m_targetDir;
};

} } // namespace
#endif // guard
