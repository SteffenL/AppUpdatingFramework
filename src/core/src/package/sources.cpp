#include "sources.h"

#include <nowide/fstream.hpp>
#include <Poco/Zip/ZipArchive.h>
#include <Poco/Zip/ZipStream.h>

namespace aufw { namespace package {

SourceBase::~SourceBase() {}

ZipFileSource::ZipFileSource(const std::string& path) {
    auto f = new nowide::ifstream(path.c_str(), std::ios::binary);
    if (!f) {
        throw std::runtime_error("Failed to allocate memory");
    }

    if (!f->is_open()) {
        throw std::runtime_error(std::string("Couldn't open file for reading: ") + path);
    }

    m_file.reset(f);

    m_zip.reset(new Poco::Zip::ZipArchive(*m_file));
    if (!m_zip) {
        throw std::runtime_error("Failed to allocate memory");
    }
}

void ZipFileSource::GetFileList(std::vector<std::string>& list) const {
    for (auto it = m_zip->headerBegin(), end = m_zip->headerEnd(); it != end; ++it) {
        list.push_back(it->first);
    }
}

std::istream* ZipFileSource::GetFile(std::string path) {
    auto it = m_zip->findHeader(path);
    if (it == m_zip->headerEnd()) {
        throw std::runtime_error(std::string("There's no such file: ") + path);
    }

    auto s = new Poco::Zip::ZipInputStream(*m_file, it->second);
    if (!s) {
        throw std::runtime_error("Failed to allocate memory");
    }

    return s;
}

} } // namespace
