#ifndef __updating_AssetsVersionCollector__
#define __updating_AssetsVersionCollector__

#include "core/PlainTextVersionCollector.h"

namespace updating {

class AssetsVersionCollector : public aufw::PlainTextVersionCollector {
public:
    AssetsVersionCollector();
    void Collect();
};

} // namespace
#endif // guard
