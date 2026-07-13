#include <etw/event_record_snapshot.h>

namespace aegis::sensor::win32::etw
{
snapshot_ptr_t make_event_record_snapshot(const EVENT_RECORD &record) noexcept
{
    auto ev = make_snapshot();

    ev->header.ev_descriptor = record.EventHeader.EventDescriptor;
    ev->header.activity_id   = record.EventHeader.ActivityId;
    ev->header.timestamp     = record.EventHeader.TimeStamp.QuadPart;
    ev->header.thread_id     = record.EventHeader.ThreadId;
    ev->header.process_id    = record.EventHeader.ProcessId;
    ev->header.flags         = record.EventHeader.Flags;
    ev->header.properties    = record.EventHeader.EventProperty;
    ev->header.provider_id   = 0 /* TODO: Create providers map */;
    if (!(record.EventHeader.Flags & EVENT_HEADER_FLAG_NO_CPUTIME))
    {
        ev->header.ku_time = EventHeader::KernelUserTime{
            .kernel_time = record.EventHeader.KernelTime,
            .user_time   = record.EventHeader.UserTime,
        };
    }
    else
    {
        ev->header.processor_time = record.EventHeader.ProcessorTime;
    }

    if (record.UserDataLength)
    {
        ev->user_data_size = record.UserDataLength;
        ev->user_data      = std::make_unique<unsigned char[]>(record.UserDataLength);
        memcpy(ev->user_data.get(), record.UserData, record.UserDataLength);
    }

    return ev;
}

} // namespace aegis::sensor::win32::etw
