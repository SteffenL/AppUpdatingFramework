#ifndef __aufw_Elevation__
#define __awuf_Elevation__

#ifdef _WIN32
#include "Elevation_msw.h"
#else
#error Not implemented
#endif

namespace aufw {

#ifdef _WIN32
typedef Elevation_msw Elevation;
#else
#error Not implemented
#endif

} // namespace
#endif // guard
