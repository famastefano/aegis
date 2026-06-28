#pragma once

#include <windows.h>
#include <evntprov.h>

#include <cstdint>

namespace aegis::sensor::win32::etw {

class SelfTestProvider final {
public:
    SelfTestProvider();
    ~SelfTestProvider() noexcept;

    SelfTestProvider(const SelfTestProvider&) = delete;
    SelfTestProvider& operator=(const SelfTestProvider&) = delete;
    SelfTestProvider(SelfTestProvider&&) = delete;
    SelfTestProvider& operator=(SelfTestProvider&&) = delete;

    ULONG write_event(std::uint32_t sequence) const noexcept;

private:
    REGHANDLE handle_{};
};

} // namespace aegis::sensor::win32::etw
