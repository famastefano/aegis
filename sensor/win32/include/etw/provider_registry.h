#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace aegis::sensor::win32::etw {

extern const GUID AegisSelfTestProviderGuid;
extern const GUID MicrosoftWindowsKernelProcessProviderGuid;

struct EtwProviderConfig {
    GUID provider_id{};
    std::wstring name;
    UCHAR level{};
    ULONGLONG match_any_keyword{};
    ULONGLONG match_all_keyword{};
};

struct ProviderSelection {
    bool enable_self_test{true};
    bool enable_kernel_process{false};
};

class EtwProviderRegistry {
public:
    [[nodiscard]] static std::vector<EtwProviderConfig> build(ProviderSelection selection);
};

} // namespace aegis::sensor::win32::etw
