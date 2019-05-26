#include <aufw/core/progress/ProgressReader.h>

// Serialization
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>

namespace aufw { namespace progress {

void ProgressReader::Load() {
    using boost::serialization::make_nvp;
    boost::archive::xml_iarchive ar(m_stream);

    ar & make_nvp("TargetDir", TargetDir);
    ar & make_nvp("BackupDir", BackupDir);
    ar & make_nvp("DownloadDir", DownloadDir);
    ar & make_nvp("Application", m_application);
    ar & make_nvp("Components", m_components);
}

ProgressReader::ProgressReader(const std::string& filePath) : m_stream(filePath.c_str(), std::ios::binary) {}

} } // namespace
