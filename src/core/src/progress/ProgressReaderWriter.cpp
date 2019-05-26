#include <aufw/core/progress/ProgressReaderWriter.h>
#include "../exceptions.h"
#include <boost/archive/archive_exception.hpp>
#include <boost/filesystem.hpp>
#include <nowide/fstream.hpp>

namespace aufw { namespace progress {

ProgressReaderWriter::ProgressReaderWriter(const std::string& filePath) : m_filePath(filePath) {}

void ProgressReaderWriter::Load() {
    std::lock_guard<std::mutex> lock(m_ioMutex);

    nowide::fstream m_stream;
    m_stream.open(m_filePath.c_str(), std::ios::in);
    if (!m_stream.is_open()) {
        throw FileException("Unable to open file", m_filePath);
    }

    try {
        using boost::serialization::make_nvp;
        boost::archive::xml_iarchive ar(m_stream);

        ar & make_nvp("TargetDir", TargetDir);
        ar & make_nvp("BackupDir", BackupDir);
        ar & make_nvp("DownloadDir", DownloadDir);

        try {
            ar & make_nvp("Application", m_application);
            m_hasApplication = true;
        }
        catch (boost::archive::archive_exception&) {
            m_hasApplication = false;
        }

        try {
            ar & make_nvp("Components", m_components);
            m_haveComponents = true;
        }
        catch (boost::archive::archive_exception&) {
            m_haveComponents = false;
        }

        m_stream.close();
    }
    catch (boost::archive::archive_exception&) {
        m_stream.close();
        throw;
    }
}

void ProgressReaderWriter::Save() {
    std::lock_guard<std::mutex> lock(m_ioMutex);

    namespace fs = boost::filesystem;
    fs::path updateBaseDir(fs::path(m_filePath).parent_path());
    if (!fs::exists(updateBaseDir)) {
        if (!fs::create_directories(updateBaseDir)) {
            throw FileException("Cannot create directories", updateBaseDir.string());
        }
    }

    nowide::fstream m_stream;
    m_stream.open(m_filePath.c_str(), std::ios::out | std::ios::trunc);
    if (!m_stream.is_open()) {
        throw FileException("Unable to open file", m_filePath);
    }

    try {
        // Make sure the archive object goes out of scope before closing the stream so that the content is properly written/closed
        {
            using boost::serialization::make_nvp;
            boost::archive::xml_oarchive ar(m_stream);

            ar & make_nvp("TargetDir", TargetDir);
            ar & make_nvp("BackupDir", BackupDir);
            ar & make_nvp("DownloadDir", DownloadDir);

            if (HasApplication()) {
                ar & make_nvp("Application", m_application);
            }

            if (HaveComponents()) {
                ar & make_nvp("Components", m_components);
            }
        }

        m_stream.close();
    }
    catch (boost::archive::archive_exception&) {
        m_stream.close();
        throw;
    }
}

std::string ProgressReaderWriter::GetFilePath() const {
    return m_filePath;
}

void ProgressReaderWriter::CleanupFiles() const {
    std::lock_guard<std::mutex> lock(m_ioMutex);

    namespace fs = boost::filesystem;
    fs::path baseDir(fs::path(m_filePath).parent_path());
    if (!fs::exists(baseDir)) {
        return;
    }

    boost::system::error_code errorCode;
    fs::remove_all(baseDir, errorCode);

    if (errorCode.value() != boost::system::errc::success) {
        throw FileException(errorCode.message(), baseDir.string());
    }
}

} } // namespace
