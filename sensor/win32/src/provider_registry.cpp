#include <etw/provider_registry.h>

#include <evntrace.h>

namespace aegis::sensor::win32::etw {

const GUID AegisSelfTestProviderGuid{
    0x0f6d7603,
    0x77b9,
    0x46ed,
    {0xa7, 0xd0, 0x04, 0xa8, 0x9d, 0x30, 0x11, 0x97},
};

const GUID MicrosoftWindowsKernelProcessProviderGuid{
    0x22fb2cd6,
    0x0e7b,
    0x422b,
    {0xa0, 0xc7, 0x2f, 0xad, 0x1f, 0xd0, 0xe7, 0x16},
};

std::vector<EtwProviderConfig> EtwProviderRegistry::build(ProviderSelection selection)
{
    std::vector<EtwProviderConfig> providers;

    if (selection.enable_self_test) {
        providers.push_back(EtwProviderConfig{
            .provider_id = AegisSelfTestProviderGuid,
            .name = L"Aegis-Sensor-Win32-SelfTest",
            .level = TRACE_LEVEL_INFORMATION,
            .match_any_keyword = 0,
            .match_all_keyword = 0,
        });
    }

    if (selection.enable_kernel_process) {
        providers.push_back(EtwProviderConfig{
            .provider_id = MicrosoftWindowsKernelProcessProviderGuid,
            .name = L"Microsoft-Windows-Kernel-Process",
            .level = TRACE_LEVEL_INFORMATION,
            .match_any_keyword = 0x10,
            .match_all_keyword = 0,
        });
    }

    return providers;
}

} // namespace aegis::sensor::win32::etw
