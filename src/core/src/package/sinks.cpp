#include "sinks.h"
#include "../exceptions.h"

#include <boost/filesystem.hpp>
#include <nowide/fstream.hpp>

namespace aufw { namespace package {

SinkBase::~SinkBase() {}

DirectorySink::DirectorySink(const std::string& path) : m_path(path) {}

void DirectorySink::Consume(SourceBase& source) {
    namespace fs = boost::filesystem;

    // Create output directory if needed
    if (!fs::exists(m_path)) {
        if (!fs::create_directory(m_path)) {
            throw FileException("Cannot create directory", m_path);
        }
    }

    std::vector<std::string> fileList;
    source.GetFileList(fileList);
    for (auto it = fileList.cbegin(), end(fileList.cend()); it != end; ++it) {
        const auto& relativeFilePath = *it;
        std::istream* stream = source.GetFile(relativeFilePath);
        if (!stream) {
            throw FileException("Cannot get file/directory from ZIP archive (corrupt?)", relativeFilePath);
        }

        std::string absoluteFilePath(m_path);
        absoluteFilePath +=  + "/";
        absoluteFilePath += relativeFilePath;

        // Directory?
        if (absoluteFilePath[absoluteFilePath.size() - 1] == '/') {
            // Create directory if needed
            if (!fs::exists(absoluteFilePath)) {
                if (!fs::create_directory(absoluteFilePath)) {
                    throw FileException("Cannot create directory", absoluteFilePath);
                }
            }
        }
        // File
        else {
            // Extract
			nowide::ofstream outFile(absoluteFilePath.c_str(), std::ios::binary);
            if (!outFile.is_open()) {
                throw FileException("Cannot create file", absoluteFilePath);
            }

            std::copy(
                std::istreambuf_iterator<char>(*stream),
                std::istreambuf_iterator<char>(),
                std::ostreambuf_iterator<char>(outFile));
            m_completeFiles.push_back(*it);
        }

        delete stream;
        stream = nullptr;
    }
}

std::vector<std::string>& DirectorySink::GetCompleteFiles() {
    return m_completeFiles;
}

} } // namespace
