#pragma once

#include <windows.h>
#include <evntcons.h>

namespace aegis::sensor::win32::etw {

struct EventRecordSnapshot {
    GUID provider_id{};
    ULONG process_id{};
    ULONG thread_id{};
    UCHAR level{};
    UCHAR opcode{};
    USHORT event_id{};
    UCHAR version{};
};

[[nodiscard]] EventRecordSnapshot make_event_record_snapshot(const EVENT_RECORD& record) noexcept;

} // namespace aegis::sensor::win32::etw
