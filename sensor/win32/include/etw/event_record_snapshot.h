#pragma once

#include <windows.h>
#include <evntcons.h>

namespace aegis::sensor::win32::etw {

struct EventRecordSnapshot {
    EVENT_RECORD ev;
};

[[nodiscard]] EventRecordSnapshot make_event_record_snapshot(const EVENT_RECORD& record) noexcept;

} // namespace aegis::sensor::win32::etw
