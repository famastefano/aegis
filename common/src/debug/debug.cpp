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
} // namespace aegis::debug