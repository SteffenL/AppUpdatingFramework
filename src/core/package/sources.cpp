#include "sources.h"

#include <io/ZIP.h>

namespace aufw { namespace package {

SourceBase::~SourceBase() {}

ZipFileSource::ZipFileSource(const std::string& path) :
    m_reader(new Partio::ZipFileReader(path)) {}

void ZipFileSource::GetFileList(std::vector<std::string>& list) const {
    m_reader->Get_File_List(list);
}

std::istream* ZipFileSource::GetFile(std::string path) {
    return m_reader->Get_File(path);
}

} } // namespace
