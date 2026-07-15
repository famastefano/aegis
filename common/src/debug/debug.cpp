#include <aegis/debug/debug.h>

#include <exception>

#ifdef AEGIS_TARGET_WINDOWS
    #include <Windows.h>
#endif

namespace aegis::debug
{
bool is_debugger_attached()
{
#ifdef AEGIS_TARGET_WINDOWS
    return IsDebuggerPresent();
#else
    return false;
#endif
}

void assert(bool expr)
{
    if (expr)
        return;

    if (is_debugger_attached())
    {
#ifdef AEGIS_TARGET_WINDOWS
        DebugBreak();
#endif
    }
    std::terminate();
}

bool is_admin()
{
#ifdef AEGIS_TARGET_WINDOWS
    HANDLE token = nullptr;
    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token);

    TOKEN_ELEVATION elevation{};
    DWORD           size = 0;

    GetTokenInformation(
        token,
        TokenElevation,
        &elevation,
        sizeof(elevation),
        &size);

    CloseHandle(token);

    return elevation.TokenIsElevated != 0;
#else
    return false;
#endif
}
} // namespace aegis::debug