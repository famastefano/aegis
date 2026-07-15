#pragma once

#include <memory>
#include <optional>
#include <string_view>

struct _PROVIDER_ENUMERATION_INFO;
struct _TRACE_PROVIDER_INFO;
struct _GUID;

namespace aegis::sensor::win32::etw
{
class EtwProviderRegistry
{
  public:
    static EtwProviderRegistry &get_registry();

    inline static constexpr std::uint16_t SELF_TEST_PROVIDER_ID = 0;

    [[maybe_unused]] std::size_t discover_providers();

    std::size_t get_providers_count() const;

    struct ProviderInfo
    {
        // Name of the provider as returned by the OS
        std::wstring_view name_{};
        // GUID of the provider as returned by the OS
        _GUID const *guid_{};
        // Internal ID that uniquely identifies this provider.
        // **WARNING**: Might change between executions.
        std::uint16_t id_{};
    };
    std::optional<ProviderInfo> try_get_provider_info(std::size_t pos) const;
    std::optional<ProviderInfo> try_get_provider_info_by_id(std::uint16_t id) const;
    std::optional<ProviderInfo> try_get_provider_info_by_name(std::wstring_view name) const;

  private:
    EtwProviderRegistry() = default;

    _PROVIDER_ENUMERATION_INFO       *get_storage();
    _PROVIDER_ENUMERATION_INFO const *get_storage() const;

    ProviderInfo extract_info(_TRACE_PROVIDER_INFO const &info, std::uint16_t id) const;

    std::unique_ptr<unsigned char[]> provider_storage_;
};

} // namespace aegis::sensor::win32::etw
