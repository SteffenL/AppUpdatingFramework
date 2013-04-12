#include "Product.h"

namespace aufw { namespace progress {

template<class Archive>
void Product::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;

    ar & make_nvp("State", State);
    ar & make_nvp("SourceFilePath", SourceFilePath);

    switch (State) {
    case State::DownloadPending:
        ar & make_nvp("Details", DownloadPendingDetails);
        break;

    case State::Downloading:
        ar & make_nvp("Details", DownloadingDetails);
        break;

    case State::UnpackPending:
        ar & make_nvp("Details", UnpackPendingDetails);
        break;

    case State::Unpacking:
        ar & make_nvp("Details", UnpackingDetails);
        break;

    case State::InstallPending:
        ar & make_nvp("Details", InstallPendingDetails);
        break;

    case State::Installing:
        ar & make_nvp("Details", InstallingDetails);
        break;

    case State::Failed:
        ar & make_nvp("Details", FailedDetails);
        break;

    case State::Complete:
        ar & make_nvp("Details", CompleteDetails);
        break;
    }
}

} } // namespace
