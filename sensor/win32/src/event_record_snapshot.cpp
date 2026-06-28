#include <etw/event_record_snapshot.h>

namespace aegis::sensor::win32::etw {

EventRecordSnapshot make_event_record_snapshot(const EVENT_RECORD& record) noexcept
{
    EventRecordSnapshot snapshot{};
    snapshot.provider_id = record.EventHeader.ProviderId;
    snapshot.process_id = record.EventHeader.ProcessId;
    snapshot.thread_id = record.EventHeader.ThreadId;
    snapshot.level = record.EventHeader.EventDescriptor.Level;
    snapshot.opcode = record.EventHeader.EventDescriptor.Opcode;
    snapshot.event_id = record.EventHeader.EventDescriptor.Id;
    snapshot.version = record.EventHeader.EventDescriptor.Version;
    return snapshot;
}

} // namespace aegis::sensor::win32::etw
