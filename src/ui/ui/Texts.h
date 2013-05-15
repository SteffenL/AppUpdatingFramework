#ifndef __aufw_ui_Texts__
#define __aufw_ui_Texts__

#include "core/progress/State.h"
#include <wx/string.h>

namespace aufw { namespace ui {

struct Texts {
    static wxString GetStateText(progress::State::type state);
};

} } // namespace
#endif // guard
