#ifndef __aufw_Process__
#define __awuf_Process__

#ifdef _WIN32
#include "Process_msw.h"
#else
#error Not implemented
#endif

namespace aufw {

#ifdef _WIN32
typedef Process_msw Process;
#else
#error Not implemented
#endif

} // namespace
#endif // guard
