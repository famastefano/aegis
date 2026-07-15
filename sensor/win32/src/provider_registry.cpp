#include <aegis/debug/debug.h>
#include <etw/provider_registry.h>

#include <memory>

#include <Windows.h>

#include <evntrace.h>
#include <tdh.h>

namespace aegis::sensor::win32::etw
{
EtwProviderRegistry &EtwProviderRegistry::get_registry()
{
    static EtwProviderRegistry instance;
    return instance;
}

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
        return storage->NumberOfProviders;
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

    if (auto *storage = get_storage(); pos < storage->NumberOfProviders)
    {
        TRACE_PROVIDER_INFO const &provider = storage->TraceProviderInfoArray[pos];
        return extract_info(provider, static_cast<std::uint16_t>(pos));
    }
    return {};
}

std::optional<EtwProviderRegistry::ProviderInfo> EtwProviderRegistry::try_get_provider_info_by_id(
    std::uint16_t id) const
{ return try_get_provider_info(id); }

std::optional<EtwProviderRegistry::ProviderInfo> EtwProviderRegistry::try_get_provider_info_by_name(std::wstring_view name) const
{
    auto const  hash_to_find = std::hash<std::wstring_view>{}(name);
    auto const *storage      = get_storage();
    for (std::size_t pos = 0; pos < storage->NumberOfProviders; ++pos)
    {
        auto info = extract_info(storage->TraceProviderInfoArray[pos], static_cast<std::uint16_t>(pos));
        if (std::hash<std::wstring_view>{}(info.name_) == hash_to_find)
            return info;
    }
    return {};
}

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

EtwProviderRegistry::ProviderInfo EtwProviderRegistry::extract_info(_TRACE_PROVIDER_INFO const &info, std::uint16_t id) const
{
    return ProviderInfo{
        .name_ = std::wstring_view(reinterpret_cast<wchar_t *>(provider_storage_.get() + info.ProviderNameOffset)),
        .guid_ = &info.ProviderGuid,
        .id_   = id,
    };
}

} // namespace aegis::sensor::win32::etw
