#include <aufw/core/UpdateInfoCollection.h>
#include <aufw/core/UpdateInfoCollectorBase.h>
#include <json/json.h>
#include <algorithm>

namespace aufw {

void UpdateInfoCollection::AddCollector(UpdateInfoCollectorBase* collector) {
    m_collectors.push_back(std::unique_ptr<UpdateInfoCollectorBase>(collector));
}

void UpdateInfoCollection::Collect() {
    std::for_each(m_collectors.begin(), m_collectors.end(), [&](const std::unique_ptr<UpdateInfoCollectorBase>& collector) {
        collector->Collect();
    });
}

void UpdateInfoCollection::GetResult(std::list<ProductInstallationInfo>& infoList) const {
    std::for_each(m_collectors.begin(), m_collectors.end(), [&](const std::unique_ptr<UpdateInfoCollectorBase>& collector) {
        ProductInstallationInfo pi;
        collector->GetResult(pi);
        infoList.push_back(pi);
    });
}

} // namespace
