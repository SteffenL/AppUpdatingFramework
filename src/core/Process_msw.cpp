#include "Process_msw.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <nowide/convert.hpp>
#include <algorithm>

namespace aufw {

unsigned long Process_msw::LaunchElevated(const std::string& path, const std::string& params, void* parentWindow) {
    SHELLEXECUTEINFOW sei;
    ::memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";
    std::wstring& pathW = nowide::widen(path);
    sei.lpFile = pathW.c_str();
    std::wstring& paramsW = nowide::widen(params);
    sei.lpParameters = paramsW.c_str();
    sei.nShow = SW_SHOWNORMAL;
    sei.hwnd = reinterpret_cast<HWND>(parentWindow);

    if (!::ShellExecuteExW(&sei)) {
        return 0;
    }

    /*if (reinterpret_cast<int>(sei.hInstApp) <= 32) {
        return 0;
    }*/

    unsigned long pid = 0;
    if (sei.hProcess) {
        pid = ::GetProcessId(sei.hProcess);
        ::CloseHandle(sei.hProcess);
    }

    return pid;
}

unsigned long Process_msw::LaunchNonElevated(const std::string& path, const std::string& params) {
    unsigned long pid = 0;
    HANDLE token = NULL;

    do {
        token = getLoggedOnUserImpersonationToken();

        // Check OS version
        OSVERSIONINFOEX oviVista = {sizeof(OSVERSIONINFOEX), 6, 1};
        bool isVistaOrLater = (::VerifyVersionInfo(&oviVista, VER_MAJORVERSION | VER_MINORVERSION,
            ::VerSetConditionMask(0, VER_MAJORVERSION | VER_MINORVERSION, VER_GREATER_EQUAL)) == TRUE);

        STARTUPINFO si;
        ::memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;

        std::wstring& pathW = nowide::widen(path);
        std::wstring& paramsW = nowide::widen(params);
        bool useFallback = false;

        if (isVistaOrLater) {
            // CreateProcessWithTokenW is available on Vista/Server 2003 and later
            typedef BOOL (WINAPI *CreateProcessWithTokenW_t)(
                HANDLE, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
            auto fnCreateProcessWithTokenW = reinterpret_cast<CreateProcessWithTokenW_t>(
                ::GetProcAddress(::LoadLibraryA("advapi32"), "CreateProcessWithTokenW"));
            if (!fnCreateProcessWithTokenW) {
                throw std::runtime_error("Couldn't get CreateProcessWithTokenW");
            }

            setProcessPrivilege(SE_IMPERSONATE_NAME, true);

            if (!fnCreateProcessWithTokenW(token, 0, pathW.c_str(), const_cast<LPWSTR>(paramsW.c_str()), 0, NULL, NULL, &si, &pi)) {
                if (::GetLastError() == ERROR_PRIVILEGE_NOT_HELD) {
                    useFallback = true;
                }
                else {
                    break;
                }
            }
        }
        else {
            // Fallback on earlier OS
            useFallback = true;
        }

        if (useFallback) {
            setProcessPrivilege(SE_INCREASE_QUOTA_NAME, true);
            setProcessPrivilege(SE_ASSIGNPRIMARYTOKEN_NAME, true);
            
            if (!::CreateProcessAsUserW(token, pathW.c_str(), const_cast<LPWSTR>(paramsW.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                return 0;
            }
        }

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
        pid = pi.dwProcessId;
    } while (false);

    if (token) {
        ::CloseHandle(token);
    }

    return pid;
}

unsigned long Process_msw::Launch(const std::string& path, const std::string& params) {
    STARTUPINFO si;
    ::memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;

    std::wstring& pathW = nowide::widen(path);
    std::wstring& paramsW = nowide::widen(params);
    if (!::CreateProcessW(pathW.c_str(), const_cast<LPWSTR>(paramsW.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return 0;
    }

    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
    auto pid = pi.dwProcessId;
	return pid;
}

void* Process_msw::getLoggedOnUserImpersonationToken() {
    auto shellWindow = ::GetShellWindow();
    if (!shellWindow) {
        throw std::runtime_error("Couldn't get shell window");
    }

    DWORD pid = 0;
    ::GetWindowThreadProcessId(shellWindow, &pid);
    if (!pid) {
        throw std::runtime_error("Couldn't get process ID");
    }

    HANDLE process = NULL;
    HANDLE shellProcessToken = NULL;
    HANDLE primaryToken = NULL;

    do {
        process = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!process) {
            break;
        }

        if (!::OpenProcessToken(process, TOKEN_DUPLICATE, &shellProcessToken)) {
            break;
        }

        if (!::DuplicateTokenEx(
            shellProcessToken,
            TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID,
            NULL, SecurityImpersonation, TokenPrimary, &primaryToken)) {
            break;
        }
    } while (false);

    if (process) {
        ::CloseHandle(process);
    }

    if (shellProcessToken) {
        ::CloseHandle(shellProcessToken);
    }

    if (!primaryToken) {
        throw std::runtime_error("Couldn't duplicate shell process token");
    }

    if (!process) {
        throw std::runtime_error("Couldn't open process");
    }

    if (!shellProcessToken) {
        throw std::runtime_error("Couldn't open shell process token");
    }

    return primaryToken;
}

void Process_msw::setTokenPrivilege(void* token, const std::wstring& name, bool enable) {
    LUID luid;
    if (!::LookupPrivilegeValueW(NULL, name.c_str(), &luid)) {
        throw std::runtime_error("LookupPrivilegeValueW failed");
    }

    TOKEN_PRIVILEGES tokenPrivileges;
    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Luid = luid;
    tokenPrivileges.Privileges[0].Attributes = (enable ? SE_PRIVILEGE_ENABLED : 0);

    if (!::AdjustTokenPrivileges(token, FALSE, &tokenPrivileges, sizeof(tokenPrivileges), NULL, NULL)) {
        throw std::runtime_error("AdjustTokenPrivileges failed");
    }

    if (::GetLastError() == ERROR_NOT_ALL_ASSIGNED){
        throw std::runtime_error("Couldn't enable privilege");
    }
}

void Process_msw::setProcessPrivilege(void* process, const std::wstring& name, bool enable) {
    HANDLE token = NULL;
    if (!::OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &token)) {
        throw std::runtime_error("OpenProcessToken failed");
    }

    try {
        setTokenPrivilege(token, name, enable);
    }
    catch (std::runtime_error&) {
        ::CloseHandle(token);
        throw;
    }

    ::CloseHandle(token);
}

void Process_msw::setProcessPrivilege(const std::wstring& name, bool enable) {
    setProcessPrivilege(::GetCurrentProcess(), name, enable);
}

unsigned long Process_msw::GetCurrentPid() {
    return ::GetCurrentProcessId();
}

void Process_msw::FindByName(const std::string& name, std::vector<unsigned long>& pidList) {
    auto snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (!::Process32FirstW(snap, &pe)) {
            break;
        }

        do {
            std::wstring nameLc(nowide::widen(name));
            std::wstring name2Lc(pe.szExeFile);
            std::transform(name.begin(), name.end(), nameLc.begin(), ::toupper);
            std::transform(name2Lc.begin(), name2Lc.end(), name2Lc.begin(), ::toupper);

            if (nameLc == name2Lc) {
                pidList.push_back(pe.th32ProcessID);
            }
        } while (::Process32NextW(snap, &pe));
    } while (false);

    ::CloseHandle(snap);
}

bool Process_msw::Terminate(unsigned long pid) {
    auto process = ::OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!process) {
        return false;
    }

    bool isOk = ::TerminateProcess(process, 0) ? true : false;
    ::WaitForSingleObject(process, 5000);
    ::CloseHandle(process);
    return isOk;
}

} // namespace
