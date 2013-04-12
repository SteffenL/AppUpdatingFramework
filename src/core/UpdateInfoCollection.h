#ifndef __aufw_webapi__UpdateInfoCollection__
#define __aufw_webapi__UpdateInfoCollection__

#include "ProductInstallationInfo.h"
#include <list>
#include <memory>

namespace Json { class Value; }

namespace aufw {

class UpdateInfoCollectorBase;

class UpdateInfoCollection {
public:
    virtual void AddCollector(UpdateInfoCollectorBase* collector);
    virtual void Collect();
    virtual void GetResult(std::list<ProductInstallationInfo>& productInfoList) const;

protected:
    std::list<std::unique_ptr<UpdateInfoCollectorBase>> m_collectors;
};

} // namespace
#endif // guard
