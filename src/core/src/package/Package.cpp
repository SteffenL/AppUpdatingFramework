#include "Package.h"

namespace aufw { namespace package {

Package::~Package() {}

bool Package::hasSource() const {
    return (m_source.get() != nullptr);
}

void Package::setSource(SourceBase* source) {
    m_source.reset(source);
}

} } // namespace
