#ifndef __aufw_package_Step__
#define __aufw_package_Step__

#include <iostream>
#include <functional>
#include <string>
#include <typeinfo>

namespace boost { namespace archive { class xml_iarchive; class xml_oarchive; } }

namespace aufw { namespace job {

class Step {
public:
    virtual ~Step();
    virtual void Execute() = 0;
    virtual void Rollback() = 0;
    virtual void Serialize(boost::archive::xml_iarchive& ar);
    virtual void Serialize(boost::archive::xml_oarchive& ar);
    std::string GetTypeName() const;
    static std::string GetTypeName_();

protected:
    virtual void onProgress();
    static void demangleTypeName(const std::string name);
    static std::string getTypeNameInternal(const type_info& typeInfo);

public:
    std::function<void (const Step&)> OnProgress;
};

} } // namespace
#endif // guard
