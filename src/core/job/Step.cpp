#include "Step.h"

namespace aufw { namespace job {

Step::~Step() {}
void Step::Serialize(boost::archive::xml_iarchive& ar) {}
void Step::Serialize(boost::archive::xml_oarchive& ar) {}

void Step::onProgress() {
    if (OnProgress) {
        OnProgress(*this);
    }
}

std::string Step::GetTypeName() const {
    return getTypeNameInternal(typeid(*this));
}

void Step::demangleTypeName(const std::string name) {
#ifndef _MSC_VER
#error May have to demangle type name
    /*
    Look here for more info:
    http://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html
    http://stackoverflow.com/questions/789402/typeid-returns-extra-characters-in-g
    Sample code:
    int status;
    char *realname = abi::__cxa_demangle(typeid(obj).name(), 0, 0, &status);
    std::cout << realname;
    free(realname);
    */
#endif
}

std::string Step::getTypeNameInternal(const type_info& typeInfo) {
    std::string name = typeInfo.name();
    demangleTypeName(name);
    return name;
}

} } // namespace
