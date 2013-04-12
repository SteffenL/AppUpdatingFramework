#ifndef __aufw_progress_details__
#define __aufw_progress_details__

#include "Product.h"

// Serialization
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

namespace aufw { namespace progress {

struct Product;

struct ProgressDetailsBase_t {

    // Serialization
    template<class Archive>
    void serialize(Archive& ar, const unsigned version) {
        using boost::serialization::make_nvp;
    }
};

struct DownloadDetails_t : public ProgressDetailsBase_t {
    uint64_t FileSize;
    uint64_t TotalDownloaded;

    DownloadDetails_t() : FileSize(0), TotalDownloaded(0) {}

    // Serialization
    template<class Archive>
    void serialize(Archive& ar, const unsigned version) {
        using boost::serialization::make_nvp;
        ProgressDetailsBase_t::serialize<Archive>(ar, version);
        ar & make_nvp("FileSize", FileSize);
        ar & make_nvp("TotalDownloaded", TotalDownloaded);
    }
};

struct VerifyDetails_t : public ProgressDetailsBase_t {
    std::string Hash;

    VerifyDetails_t() {}

    // Serialization
    template<class Archive>
    void serialize(Archive& ar, const unsigned version) {
        using boost::serialization::make_nvp;
        ProgressDetailsBase_t::serialize<Archive>(ar, version);
        ar & make_nvp("Hash", Hash);
    }
};

struct UnpackDetails_t : public ProgressDetailsBase_t {
    std::vector<std::string> CompletedFiles;

    // Serialization
    template<class Archive>
    void serialize(Archive& ar, const unsigned version) {
        using boost::serialization::make_nvp;
    }
};

struct InstallDetails_t : public ProgressDetailsBase_t {};

struct CompleteDetails_t {
    // Serialization
    template<class Archive>
    void serialize(Archive& ar, const unsigned version) {
        using boost::serialization::make_nvp;
    }
};

} } // namespace
#endif // guard
