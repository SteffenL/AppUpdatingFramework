#include "UnpackFilePackageStep.h"

#include "../package/sources.h"
#include "../package/sinks.h"
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

UnpackFilePackageStep::UnpackFilePackageStep() : m_package(nullptr) {}

UnpackFilePackageStep::UnpackFilePackageStep(package::FilePackage* package, const std::string& targetDir, const std::string& backupDir) :
    m_package(package), m_targetDir(targetDir), m_backupDir(backupDir) {}

void UnpackFilePackageStep::Init(package::FilePackage* package, const std::string& targetDir, const std::string& backupDir) {
    m_package = package;
    m_targetDir = targetDir;
    m_backupDir = backupDir;
}

void UnpackFilePackageStep::Execute() {
    package::DirectorySink sink(m_targetDir);

    package::SourceBase& source = m_package->GetSource();
    source.GetFileList(m_fileList);

    try {
        sink.Consume(source);
        // Track files
        m_completeFiles = sink.GetCompleteFiles();
    }
    catch (std::exception&) {
        // Track files
        m_completeFiles = sink.GetCompleteFiles();
        throw;
    }
}

void UnpackFilePackageStep::Rollback() {
    namespace fs = boost::filesystem;

    // Delete unpacked files
    for (auto it = m_fileList.rbegin(), end(m_fileList.rend()); it != end; ++it) {
        std::string path(m_targetDir);
        path += "/";
        path += *it;

        if (!fs::exists(path)) {
            continue;
        }

        // Folder?
        if (path[path.size() - 1] == '/') {
            if (fs::is_empty(path)) {
                if (!fs::remove(path)) {
                    throw FileException("Cannot delete directory", path);
                }
            }
        }
        // File
        else {
            if (!fs::remove(path)) {
                throw FileException("Cannot delete file", path);
            }
        }
    }

    // Delete target folder
    if (fs::is_empty(m_targetDir)) {
        if (!fs::remove(m_targetDir)) {
            throw FileException("Cannot delete directory", m_targetDir);
        }
    }
}

void UnpackFilePackageStep::Serialize(boost::archive::xml_iarchive& ar) {
    using boost::serialization::make_nvp;
    ar & make_nvp("UnpackedFiles", m_completeFiles);
}

void UnpackFilePackageStep::Serialize(boost::archive::xml_oarchive& ar) {
    using boost::serialization::make_nvp;
    ar & make_nvp("UnpackedFiles", m_completeFiles);
}

std::string UnpackFilePackageStep::GetTypeName_() {
    return Step::getTypeNameInternal(typeid(UnpackFilePackageStep));
}

} } // namespace
