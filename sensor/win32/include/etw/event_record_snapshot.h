#pragma once

#include <etw/events/extended_data.h>

#include <memory>

namespace aegis::sensor::win32::etw
{

// Packed EVENT_HEADER snapshot
struct EventHeader
{
    struct KernelUserTime
    {
        std::uint32_t kernel_time;
        std::uint32_t user_time;
    };

    EVENT_DESCRIPTOR ev_descriptor;
    GUID             activity_id;
    std::uint64_t    timestamp;
    union {
        KernelUserTime ku_time;
        std::uint64_t  processor_time;
    };
    std::uint32_t thread_id;
    std::uint32_t process_id;
    std::uint16_t flags : 10;
    std::uint16_t properties : 4;
    // Converted provider GUID to an internal identifier
    std::uint16_t provider_id;
};

// Packed EVENT_RECORD + RAII-safe deep copy of the data
struct EventRecordSnapshot
{
    EventHeader                      header;
    std::unique_ptr<unsigned char[]> user_data;
    std::unique_ptr<ExtendedData[]>  ext_data;
    std::uint16_t                    user_data_size;
    std::uint16_t                    ext_data_size;
};

[[nodiscard]] EventRecordSnapshot make_event_record_snapshot(const EVENT_RECORD &record) noexcept;

} // namespace aegis::sensor::win32::etw
