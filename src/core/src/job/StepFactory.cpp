#include "StepFactory.h"
#include <aufw/core/job/Step.h>
#include "MoveFilesStep.h"
#include "UnpackFilePackageStep.h"
#include "StartProgramStep.h"

#include <cassert>
#include <stdexcept>

namespace aufw { namespace job {

Step* StepFactory::CreateFromName(const std::string& name) {
    if (name == MoveFilesStep::GetTypeName_()) {
        return new MoveFilesStep;
    }
    else if (name == UnpackFilePackageStep::GetTypeName_()) {
        return new UnpackFilePackageStep;
    }
    else if (name == StartProgramStep::GetTypeName_()) {
        return new StartProgramStep;
    }
    else {
        assert(!"Unknown type name");
        throw std::logic_error("Unknown type name");
    }
}

} } // namespace
