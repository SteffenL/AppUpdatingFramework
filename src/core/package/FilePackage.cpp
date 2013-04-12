#include "FilePackage.h"

namespace aufw { namespace package {

FilePackage::FilePackage(const std::string& path) : m_path(path) {}
FilePackage::~FilePackage() {}

SourceBase& FilePackage::GetSource() {
    if (!hasSource()) {
        setSource(new ZipFileSource(m_path));
    }

    return *m_source;
}

} } // namespace
