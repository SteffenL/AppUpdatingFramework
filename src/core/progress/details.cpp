#include "details.h"

namespace aufw { namespace progress {

template<class Archive>
void DownloadPendingDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

template<class Archive>
void DownloadingDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

template<class Archive>
void UnpackPendingDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

template<class Archive>
void UnpackingDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

template<class Archive>
void InstallPendingDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

template<class Archive>
void InstallingDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

template<class Archive>
void CompleteDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

template<class Archive>
void FailedDetails_t::serialize(Archive& ar, const unsigned version)
{
    using boost::serialization::make_nvp;
}

} } // namespace
