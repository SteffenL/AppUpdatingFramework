#ifndef __aufw_progress_Product__
#define __aufw_progress_Product__

#include "../ProductUpdateDetails.h"
#include "details.h"
#include "State.h"
#include "../job/Job.h"

// Serialization
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>

#include <iostream>
#include <string>

namespace aufw { struct ProductUpdateDetails; }

namespace aufw { namespace progress {

struct Details_t {
    DownloadDetails_t Download;
    VerifyDetails_t Verify;
    UnpackDetails_t Unpack;
    InstallDetails_t Install;
    CompleteDetails_t Complete;
};

struct Product {
    State::type State;
    ProductUpdateDetails UpdateDetails;
    std::string SourceFilePath;
    Details_t ProgressDetails;
    std::string TempFilePath;
    Version InstalledVersion;
    job::Job* Job;

    Product() : Job(nullptr) {}

    // Serialization
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        using boost::serialization::make_nvp;

        ar & make_nvp("State", State);
        ar & make_nvp("SourceFilePath", SourceFilePath);
        ar & make_nvp("UpdateDetails", UpdateDetails);
        ar & make_nvp("TempFilePath", TempFilePath);
        ar & make_nvp("InstalledVersion", InstalledVersion.ToString());

        // Save job if needed
        if ((State > State::InstallPending) && (State < State::InstallComplete)) {
            if (Job) {
                Job->Serialize(ar);
            }
        }

#if 0
        switch (State) {
        case State::DownloadPending:
        case State::Downloading:
        case State::DownloadComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Download);
            break;

        case State::VerifyPending:
        case State::Verifying:
        case State::VerifyComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Verify);
            break;

        case State::UnpackPending:
        case State::Unpacking:
        case State::UnpackComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Unpack);
            break;

        case State::InstallPending:
        case State::Installing:
        case State::InstallComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Install);
            break;

        case State::Complete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Complete);
            break;
        }
#endif
    }

    template<class Archive>
    void load(Archive& ar, const unsigned version) {
        using boost::serialization::make_nvp;

        ar & make_nvp("State", State);
        ar & make_nvp("SourceFilePath", SourceFilePath);
        ar & make_nvp("UpdateDetails", UpdateDetails);
        ar & make_nvp("TempFilePath", TempFilePath);

        std::string installedVersionStr;
        ar & make_nvp("InstalledVersion", installedVersionStr);
        if (!InstalledVersion.FromString(installedVersionStr)) {
            throw std::runtime_error("Invalid version string");
        }

        // Load job if needed
        if ((State > State::InstallPending) && (State < State::InstallComplete)) {
            auto tempJob = new aufw::job::Job;
            try {
                tempJob->Serialize(ar);
                Job = tempJob;
                tempJob = nullptr;
            }
            catch (boost::archive::archive_exception&) {
                delete tempJob;
                throw;
            }
        }

#if 0
        switch (State) {
        case State::DownloadPending:
        case State::Downloading:
        case State::DownloadComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Download);
            break;

        case State::VerifyPending:
        case State::Verifying:
        case State::VerifyComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Verify);
            break;

        case State::UnpackPending:
        case State::Unpacking:
        case State::UnpackComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Unpack);
            break;

        case State::InstallPending:
        case State::Installing:
        case State::InstallComplete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Install);
            break;

        case State::Complete:
            ar & make_nvp("ProgressDetails", ProgressDetails.Complete);
            break;
        }
#endif
    }
};

} } // namespace
#endif // guard
