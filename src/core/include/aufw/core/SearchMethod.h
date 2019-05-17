#ifndef __aufw_SearchMethod__
#define __aufw_SearchMethod__

namespace aufw {

struct SearchMethod
{
    enum type
    {
        // Compare this version with the latest release
        CompareLatest,
        // Just find the latest release
        Latest
    };
};

} // namespace
#endif // guard
