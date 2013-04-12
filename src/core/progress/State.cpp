#include "State.h"

namespace aufw { namespace progress {

std::string State::GetStateText(type state) {
    const std::string texts[] = {
        "Download pending",
        "Downloading",
        "Download failed",
        "Download complete",
        "Verify pending",
        "Verifying",
        "Verify failed",
        "Verify complete",
        "Unpack pending",
        "Unpacking",
        "Unpack failed",
        "Unpack complete",
        "Install pending",
        "Installing",
        "Install failed",
        "Install complete",
        "Complete"
    };

    return texts[state];
}

} } // namespace
