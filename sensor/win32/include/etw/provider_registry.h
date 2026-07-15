#pragma once

#include <memory>
#include <optional>
#include <string_view>

struct _PROVIDER_ENUMERATION_INFO;
struct _GUID;

namespace aegis::sensor::win32::etw
{
class EtwProviderRegistry
{
  public:
    inline static constexpr std::uint16_t SELF_TEST_PROVIDER_ID = 0;

    [[maybe_unused]] std::size_t discover_providers();

    std::size_t get_providers_count() const;

    struct ProviderInfo
    {
        std::wstring_view name_{};
        _GUID const      *guid_{};
        std::uint16_t     id_{};
    };
    std::optional<ProviderInfo> try_get_provider_info(std::size_t pos) const;
    std::optional<ProviderInfo> try_get_provider_info_by_id(std::uint16_t id) const;

  private:
    _PROVIDER_ENUMERATION_INFO       *get_storage();
    _PROVIDER_ENUMERATION_INFO const *get_storage() const;

    std::unique_ptr<unsigned char[]> provider_storage_;
};

} // namespace aegis::sensor::win32::etw
