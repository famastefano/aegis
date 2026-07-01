#include <etw/event_record_snapshot.h>

namespace aegis::sensor::win32::etw
{

EventRecordSnapshot make_event_record_snapshot(const EVENT_RECORD &record) noexcept
{
    EventRecordSnapshot snapshot{.ev = record};
    return snapshot;
}

} // namespace aegis::sensor::win32::etw
