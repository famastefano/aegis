#include <etw/self_test_provider.h>

#include <etw/provider_registry.h>

#include <evntrace.h>

#include <system_error>

namespace aegis::sensor::win32::etw {
namespace {

constexpr EVENT_DESCRIPTOR SelfTestEventDescriptor{
    1,
    0,
    0,
    TRACE_LEVEL_INFORMATION,
    0,
    0,
    0,
};

} // namespace

SelfTestProvider::SelfTestProvider()
{
    const auto status = EventRegister(&AegisSelfTestProviderGuid, nullptr, nullptr, &handle_);
    if (status != ERROR_SUCCESS) {
        throw std::system_error{
            static_cast<int>(status),
            std::system_category(),
            "EventRegister failed for the Aegis self-test provider",
        };
    }
}

SelfTestProvider::~SelfTestProvider() noexcept
{
    if (handle_ != 0) {
        EventUnregister(handle_);
    }
}

ULONG SelfTestProvider::write_event(std::uint32_t sequence) const noexcept
{
    EVENT_DATA_DESCRIPTOR data{};
    EventDataDescCreate(&data, &sequence, sizeof(sequence));
    return EventWrite(handle_, &SelfTestEventDescriptor, 1, &data);
}

} // namespace aegis::sensor::win32::etw
