#pragma once

#include <windows.h>

#include <cstdint>
#include <evntcons.h>
#include <type_traits>

namespace aegis::sensor::win32::etw
{
// EVENT_HEADER_EXTENDED_DATA_ITEM snapshot.
// Data stored in-place to prevent additional indirection.
class ExtendedData
{
  public:
    enum class Type : std::uint8_t
    {
        // Unsupported type, no data will be provided
        NONE,
        RELATED_ACTIVITY_ID,
        SID,
        TS_ID,
        INSTANCE_INFO,
        EVENT_KEY,
        PROCESS_START_KEY,
    };

    explicit ExtendedData(EVENT_HEADER_EXTENDED_DATA_ITEM const &item);

    Type type() const
    { return type_; }

    template <typename TExtData> TExtData const *try_get_data() const
    {
        if constexpr (std::is_same_v<TExtData, EVENT_EXTENDED_ITEM_INSTANCE>)
        {
            if (type() == Type::INSTANCE_INFO)
                return &data_.instance;
        }
        else if constexpr (std::is_same_v<TExtData, EVENT_EXTENDED_ITEM_RELATED_ACTIVITYID>)
        {
            if (type() == Type::RELATED_ACTIVITY_ID)
                return &data_.rel_activity_id;
        }
        else if constexpr (std::is_same_v<TExtData, EVENT_EXTENDED_ITEM_TS_ID>)
        {
            if (type() == Type::TS_ID)
                return &data_.ts_id;
        }
        else if constexpr (std::is_same_v<TExtData, EVENT_EXTENDED_ITEM_EVENT_KEY>)
        {
            if (type() == Type::EVENT_KEY)
                return &data_.event_key;
        }
        else if constexpr (std::is_same_v<TExtData, EVENT_EXTENDED_ITEM_PROCESS_START_KEY>)
        {
            if (type() == Type::PROCESS_START_KEY)
                return &data_.process_key;
        }
        else if constexpr (std::is_same_v<TExtData, SID>)
        {
            if (type() == Type::SID)
                return &data_.sid;
        }
        return nullptr;
    }

  private:
    union {
        EVENT_EXTENDED_ITEM_INSTANCE           instance;
        EVENT_EXTENDED_ITEM_RELATED_ACTIVITYID rel_activity_id;
        EVENT_EXTENDED_ITEM_TS_ID              ts_id;
        EVENT_EXTENDED_ITEM_EVENT_KEY          event_key;
        EVENT_EXTENDED_ITEM_PROCESS_START_KEY  process_key;
        SID                                    sid; // TODO: Refactor to be easier to pool allocate
    } data_;
    Type type_ = Type::NONE;
};
} // namespace aegis::sensor::win32::etw