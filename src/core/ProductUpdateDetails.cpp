#include "ProductUpdateDetails.h"

namespace aufw {

template<class Archive>
void ProductUpdateDetails::save(Archive& ar, const unsigned int version) const {
    using boost::serialization::make_nvp;
    ar & make_nvp("UniqueKey", UniqueKey);
    ar & make_nvp("Version", Version.ToString());
    ar & make_nvp("DisplayName", DisplayName);
    ar & make_nvp("Description", Description);
    ar & make_nvp("InfoUrl", InfoUrl);
    ar & make_nvp("ManualDownloadUrl", ManualDownloadUrl);
    ar & make_nvp("UpdateDownloadUrl", UpdateDownloadUrl);
    ar & make_nvp("ReleaseNotesUrl", ReleaseNotesUrl);
    ar & make_nvp("Hash", Hash);
}

template<class Archive>
void ProductUpdateDetails::load(Archive& ar, const unsigned version) {
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
    ar & make_nvp("Hash", Hash);
}

}
