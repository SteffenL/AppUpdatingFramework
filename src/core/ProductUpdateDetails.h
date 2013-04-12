#ifndef __aufw_ProductUpdateDetails__
#define __aufw_ProductUpdateDetails__

#include "Version.h"

// Serialization
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>

#include <string>

namespace aufw {

struct ProductUpdateDetails {
    std::string UniqueKey;
    Version Version;
    std::string DisplayName;
    std::string Description;
    std::string InfoUrl;
    std::string ManualDownloadUrl;
    std::string UpdateDownloadUrl;
    std::string ReleaseNotesUrl;
    std::string UpdateFileHash;
    unsigned int UpdateFileSize;

    // Serialization
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        using boost::serialization::make_nvp;
        ar & make_nvp("UniqueKey", UniqueKey);
        ar & make_nvp("Version", Version.ToString());
        ar & make_nvp("DisplayName", DisplayName);
        ar & make_nvp("Description", Description);
        ar & make_nvp("InfoUrl", InfoUrl);
        ar & make_nvp("ManualDownloadUrl", ManualDownloadUrl);
        ar & make_nvp("UpdateDownloadUrl", UpdateDownloadUrl);
        ar & make_nvp("ReleaseNotesUrl", ReleaseNotesUrl);
        ar & make_nvp("UpdateFileHash", UpdateFileHash);
        ar & make_nvp("UpdateFileSize", UpdateFileSize);
    }

    template<class Archive>
    void load(Archive& ar, const unsigned version) {
        using boost::serialization::make_nvp;
        ar & make_nvp("UniqueKey", UniqueKey);
        
        std::string productVersionStr;
        ar & make_nvp("Version", productVersionStr);
        if (!Version.FromString(productVersionStr)) {
            throw std::runtime_error("Invalid version string");
        }

        ar & make_nvp("DisplayName", DisplayName);
        ar & make_nvp("Description", Description);
        ar & make_nvp("InfoUrl", InfoUrl);
        ar & make_nvp("ManualDownloadUrl", ManualDownloadUrl);
        ar & make_nvp("UpdateDownloadUrl", UpdateDownloadUrl);
        ar & make_nvp("ReleaseNotesUrl", ReleaseNotesUrl);
        ar & make_nvp("UpdateFileHash", UpdateFileHash);
        ar & make_nvp("UpdateFileSize", UpdateFileSize);
    }
};

} // namespace
#endif // guard
