#ifndef __aufw_webapi__PlainTextVersionCollector__
#define __aufw_webapi__PlainTextVersionCollector__

#include "Version.h"
#include "UpdateInfoCollectorBase.h"
#include <string>

namespace aufw {

class PlainTextVersionCollector : public UpdateInfoCollectorBase {
public:
    PlainTextVersionCollector(const std::string& appName);
    PlainTextVersionCollector(const std::string& appName, const std::string& filePath);
    void Collect();

protected:
    void setFilePath(const std::string& filePath);

private:
    std::string m_filePath;
};

} // namespace
#endif // guard
