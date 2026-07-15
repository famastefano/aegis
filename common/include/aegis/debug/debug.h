#pragma once

#include <aegis_common_export.h>

namespace aegis::debug
{
    AEGIS_COMMON_EXPORT bool is_debugger_attached();

    AEGIS_COMMON_EXPORT void assert(bool expr);

    AEGIS_COMMON_EXPORT bool is_admin();
}