#ifndef __aufw_progress_State__
#define __aufw_progress_State__

#include <string>

namespace aufw { namespace progress {

struct State {
    enum type {
        DownloadPending,
        Downloading,
        DownloadFailed,
        DownloadComplete,
        VerifyPending,
        Verifying,
        VerifyFailed,
        VerifyComplete,
        UnpackPending,
        Unpacking,
        UnpackFailed,
        UnpackComplete,
        InstallPending,
        Installing,
        InstallFailed,
        InstallComplete,
        Complete
    };
};

} } // namespace
#endif // guard
