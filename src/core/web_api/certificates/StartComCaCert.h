#ifndef __aufw_StartComCaCert__
#define __aufw_StartComCaCert__

#include <string>

namespace aufw { namespace web_api { namespace certificates {

class StartComCaCert {
public:
    const unsigned char* GetData() const;
    unsigned int GetSize() const;
private:
    static const unsigned char m_cacert_StartCom_pem[];
};

} } } // namespace
#endif // guard
