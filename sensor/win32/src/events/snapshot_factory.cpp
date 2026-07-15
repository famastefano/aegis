#include <aegis/allocators/pool_allocator.h>
#include <aegis/debug/debug.h>
#include <etw/events/handle.h>
#include <etw/events/snapshot.h>
#include <etw/events/snapshot_factory.h>

#include <concurrentqueue/concurrentqueue.h>

#include <mutex>

namespace aegis::sensor::win32::etw
{
namespace snapshot
{
allocators::UnorderedPoolAllocator<EventSnapshot> pool{256};
std::mutex                                        mtx;

// Snapshots we can reuse without passing by the pool allocator
moodycamel::ConcurrentQueue<SnapshotHandle> to_be_recycled;

void populate(EventSnapshot &snapshot, const _EVENT_RECORD &record);
} // namespace snapshot

SnapshotHandle make_snapshot(const _EVENT_RECORD &record)
{
    SnapshotHandle hnd;
    if (!snapshot::to_be_recycled.try_dequeue(hnd))
    {
        std::scoped_lock lck{snapshot::mtx};
        hnd.id_ = new (snapshot::pool.acquire()) EventSnapshot();
    }
    snapshot::populate(resolve_snapshot(hnd), record);
    return hnd;
}

EventSnapshot &resolve_snapshot(SnapshotHandle handle)
{
    return *std::launder(reinterpret_cast<EventSnapshot *>(handle.id_));
}

void delete_snapshot(SnapshotHandle handle)
{
    if (!snapshot::to_be_recycled.enqueue(handle))
    {
        std::scoped_lock lck{snapshot::mtx};
        snapshot::pool.release(&resolve_snapshot(handle));
    }
}

void destroy_factory()
{
    std::scoped_lock lck{snapshot::mtx};
    SnapshotHandle   hnd;
    while (snapshot::to_be_recycled.try_dequeue(hnd))
        snapshot::pool.release(&resolve_snapshot(hnd));
}

} // namespace aegis::sensor::win32::etw

namespace aegis::sensor::win32::etw::snapshot
{
void populate(EventSnapshot &ev, const _EVENT_RECORD &record)
{
    ev.header.ev_descriptor = record.EventHeader.EventDescriptor;
    ev.header.activity_id   = record.EventHeader.ActivityId;
    ev.header.timestamp     = record.EventHeader.TimeStamp.QuadPart;
    ev.header.thread_id     = record.EventHeader.ThreadId;
    ev.header.process_id    = record.EventHeader.ProcessId;
    ev.header.flags         = record.EventHeader.Flags;
    ev.header.properties    = record.EventHeader.EventProperty;
    ev.header.provider_id   = 0 /* TODO: Create providers map */;
    if (!(record.EventHeader.Flags & EVENT_HEADER_FLAG_NO_CPUTIME))
    {
        ev.header.ku_time = EventHeader::KernelUserTime{
            .kernel_time = record.EventHeader.KernelTime,
            .user_time   = record.EventHeader.UserTime,
        };
    }
    else
    {
        ev.header.processor_time = record.EventHeader.ProcessorTime;
    }

    if (record.UserDataLength)
    {
        ev.user_data_size = record.UserDataLength;
        ev.user_data      = std::make_unique<unsigned char[]>(record.UserDataLength);
        memcpy(ev.user_data.get(), record.UserData, record.UserDataLength);
    }
}

} // namespace aegis::sensor::win32::etw::snapshot
