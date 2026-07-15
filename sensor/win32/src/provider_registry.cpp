#include <aegis/debug/debug.h>
#include <etw/provider_registry.h>

#include <memory>

#include <Windows.h>

#include <evntrace.h>
#include <tdh.h>

namespace aegis::sensor::win32::etw
{
std::size_t EtwProviderRegistry::discover_providers()
{
    if (provider_storage_)
        return get_providers_count();

    ULONG sz  = 0;
    auto  ret = TdhEnumerateProviders(nullptr, &sz);
    debug::assert(ret == ERROR_INSUFFICIENT_BUFFER);
    if (ret == ERROR_INSUFFICIENT_BUFFER)
    {
        provider_storage_ = std::make_unique_for_overwrite<unsigned char[]>(sz);
        auto *storage     = std::launder(reinterpret_cast<_PROVIDER_ENUMERATION_INFO *>(provider_storage_.get()));
        ret               = TdhEnumerateProviders(storage, &sz);
        debug::assert(ret == ERROR_SUCCESS);
        if (ret != ERROR_SUCCESS)
            provider_storage_.reset();
    }

    return get_providers_count();
}

std::size_t EtwProviderRegistry::get_providers_count() const
{
    if (auto *storage = get_storage())
        return storage->NumberOfProviders + 1ull;
    return 0;
}

std::optional<EtwProviderRegistry::ProviderInfo> EtwProviderRegistry::try_get_provider_info(std::size_t pos) const
{
    if (pos == SELF_TEST_PROVIDER_ID)
    {
        static GUID null_guid{};
        return ProviderInfo{
            .name_ = L"AegisEtwSelfTestProvider",
            .guid_ = &null_guid,
            .id_   = SELF_TEST_PROVIDER_ID,
        };
    }

    if (auto *storage = get_storage(); storage->NumberOfProviders + 1 < pos)
    {
        TRACE_PROVIDER_INFO const &provider = storage->TraceProviderInfoArray[pos - 1];
        return ProviderInfo{
            .name_ = std::wstring_view(
                std::launder(reinterpret_cast<wchar_t const *>(provider_storage_.get() + provider.ProviderNameOffset))),
            .guid_ = &provider.ProviderGuid,
            .id_   = static_cast<std::uint16_t>(pos),
        };
    }
    return {};
}

std::optional<EtwProviderRegistry::ProviderInfo> EtwProviderRegistry::try_get_provider_info_by_id(
    std::uint16_t id) const
{ return try_get_provider_info(id); }

_PROVIDER_ENUMERATION_INFO *EtwProviderRegistry::get_storage()
{
    return provider_storage_ ? std::launder(reinterpret_cast<_PROVIDER_ENUMERATION_INFO *>(provider_storage_.get()))
                             : nullptr;
}

_PROVIDER_ENUMERATION_INFO const *EtwProviderRegistry::get_storage() const
{
    return provider_storage_
               ? std::launder(reinterpret_cast<_PROVIDER_ENUMERATION_INFO const *>(provider_storage_.get()))
               : nullptr;
}

} // namespace aegis::sensor::win32::etw
