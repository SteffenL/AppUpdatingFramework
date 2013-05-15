#include "Texts.h"
#include <wx/intl.h>

namespace aufw { namespace ui {

wxString Texts::GetStateText(progress::State::type state) {
    const std::string texts[] = {
        wxTRANSLATE("Download pending"),
        wxTRANSLATE("Downloading"),
        wxTRANSLATE("Download failed"),
        wxTRANSLATE("Download complete"),
        wxTRANSLATE("Verify pending"),
        wxTRANSLATE("Verifying"),
        wxTRANSLATE("Verify failed"),
        wxTRANSLATE("Verify complete"),
        wxTRANSLATE("Unpack pending"),
        wxTRANSLATE("Unpacking"),
        wxTRANSLATE("Unpack failed"),
        wxTRANSLATE("Unpack complete"),
        wxTRANSLATE("Install pending"),
        wxTRANSLATE("Installing"),
        wxTRANSLATE("Install failed"),
        wxTRANSLATE("Install complete"),
        wxTRANSLATE("Complete")
    };

    return ::wxGetTranslation(texts[state]);
}

} } // namespace
