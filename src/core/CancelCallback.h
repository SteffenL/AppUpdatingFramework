#ifndef __CancelCallback__
#define __CancelCallback__

#include <functional>

namespace aufw {

// cancel
typedef std::function<bool ()> CancelCallback_t;

} // namespace
#endif // guard
