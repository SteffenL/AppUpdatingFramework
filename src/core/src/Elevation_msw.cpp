#include <aufw/core/Elevation_msw.h>
#include <Windows.h>

namespace aufw {

bool Elevation_msw::IsUserAdmin() {
    SID_IDENTIFIER_AUTHORITY ntAuth = SECURITY_NT_AUTHORITY;
    PSID sid = NULL;
    BOOL isMember;

    if (!AllocateAndInitializeSid(
        &ntAuth,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &sid)) {
        throw std::runtime_error("AllocateAndInitializeSid failed");
    }

    if (!::CheckTokenMembership(NULL, sid, &isMember)) {
        throw std::runtime_error("CheckTokenMembership failed");
    }

    if (sid) {
        ::FreeSid(sid);
        sid = NULL;
    }

    return (isMember == TRUE);
}

} // namespace
