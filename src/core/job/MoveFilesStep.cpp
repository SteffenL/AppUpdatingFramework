#include "MoveFilesStep.h"
#include "../exceptions.h"

#include <boost/filesystem.hpp>

// Serialization
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>

namespace aufw { namespace job {

MoveFilesStep::MoveFilesStep() {}

MoveFilesStep::MoveFilesStep(const std::string& baseDir, const std::vector<std::string>& fileList, const std::string& targetDir) :
    m_baseDir(baseDir), m_fileList(fileList), m_targetDir(targetDir) {}

void MoveFilesStep::Init(const std::string& baseDir, const std::vector<std::string>& fileList, const std::string& targetDir) {
    m_baseDir = baseDir;
    m_fileList = fileList;
    m_targetDir = targetDir;
}

void MoveFilesStep::Execute() {
    namespace fs = boost::filesystem;

    // Create target base dir
    if (!fs::exists(m_targetDir)) {
        if (!fs::create_directories(m_targetDir)) {
            throw FileException("Cannot create directory", m_targetDir);
        }
    }

    for (auto it = m_fileList.begin(), end(m_fileList.end()); it != end; ++it) {
        std::string sourcePath(m_baseDir);
        sourcePath += "/";
        sourcePath += *it;
        std::string targetPath(m_targetDir);
        targetPath += "/";
        targetPath += *it;

        // Move file

        // Directory?
        if (*targetPath.rbegin() == '/') {
            // Make sure it's empty so we don't move files unintentionally
            /*if (fs::is_empty(targetPath)) {
                fs::rename(sourcePath, targetPath);
            }*/
        }
        // File
        else {
            // Skip the file if it was already completed
            if (std::find(m_completeFiles.begin(), m_completeFiles.end(), *it) != m_completeFiles.end()) {
                continue;
            }

            if (!fs::exists(sourcePath)) {
                continue;
            }

            // Delete existing target file
            fs::path p(targetPath);
            std::string dir(p.parent_path().string());
            // Create parent dir if needed
            if (!fs::exists(dir)) {
                fs::create_directories(dir);
            }

            if (fs::exists(targetPath)) {
                if (!fs::remove(targetPath)) {
                    throw FileException("Cannot delete existing target file", targetPath);
                }
            }

            try {
                // Rename file
                fs::rename(sourcePath, targetPath);
                // Track file
                m_completeFiles.push_back(*it);
            }
            catch (boost::filesystem::filesystem_error&) {
                throw;
            }
        }
    }
}

void MoveFilesStep::Rollback() {
    namespace fs = boost::filesystem;

    // Create target base dir
    if (!fs::exists(m_baseDir)) {
        if (!fs::create_directories(m_baseDir)) {
            throw FileException("Cannot create directory", m_baseDir);
        }
    }

    for (auto it = m_fileList.rbegin(), end(m_fileList.rend()); it != end; ++it) {
        std::string sourcePath(m_targetDir);
        sourcePath += "/";
        sourcePath += *it;
        std::string targetPath(m_baseDir);
        targetPath += "/";
        targetPath += *it;

        // Move file

        // Directory?
        if (*targetPath.rbegin() == '/') {
            // Make sure it's empty so we don't move files unintentionally
            /*if (fs::is_empty(targetPath)) {
                fs::rename(sourcePath, targetPath);
            }*/
        }
        // File
        else {
            // Skip the file if it was not completed
            if (std::find(m_completeFiles.begin(), m_completeFiles.end(), *it) == m_completeFiles.end()) {
                continue;
            }

            if (!fs::exists(sourcePath)) {
                m_completeFiles.pop_back();
                continue;
            }

            // Delete existing target file
            fs::path p(targetPath);
            std::string dir(p.parent_path().string());
            // Create parent dir if needed
            if (!fs::exists(dir)) {
                fs::create_directories(dir);
            }

            if (fs::exists(targetPath)) {
                if (!fs::remove(targetPath)) {
                    throw FileException("Cannot delete existing target file", targetPath);
                }
            }

            try {
                // Rename file
                fs::rename(sourcePath, targetPath);
                // Track file
                m_completeFiles.pop_back();
            }
            catch (boost::filesystem::filesystem_error&) {
                throw;
            }

            // Delete existing source directory
            fs::path p2(sourcePath);
            std::string dir2(p2.parent_path().string());
            if (fs::is_empty(dir2)) {
                fs::remove(dir2);
            }
        }
    }
}

void MoveFilesStep::Serialize(boost::archive::xml_iarchive& ar) {
    using boost::serialization::make_nvp;
    ar & make_nvp("MovedFiles", m_completeFiles);
}

void MoveFilesStep::Serialize(boost::archive::xml_oarchive& ar) {
    using boost::serialization::make_nvp;
    ar & make_nvp("MovedFiles", m_completeFiles);
}

std::string MoveFilesStep::GetTypeName_() {
    return Step::getTypeNameInternal(typeid(MoveFilesStep));
}

} } // namespace
