#include "StartComCaCert.h"

namespace aufw { namespace web_api { namespace certificates {

#include "cacert_StartCom.min.pem.h"

const unsigned char* StartComCaCert::GetData() const {
    return &m_cacert_StartCom_pem[0];
}

unsigned int StartComCaCert::GetSize() const {
    return sizeof(m_cacert_StartCom_pem);
}

} } } // namespace
