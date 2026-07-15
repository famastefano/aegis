#include <aegis/allocators/pool_allocator.h>
#include <aegis/debug/debug.h>
#include <etw/allocators/event_record_allocator.h>
#include <etw/allocators/handle.h>
#include <etw/event_record_snapshot.h>

#include <mutex>

namespace aegis::sensor::win32::etw
{
namespace
{
allocators::UnorderedPoolAllocator<EventRecordSnapshot> snapshot_allocator{256, 4};
std::mutex                                              snapshot_mutex;
} // namespace

SnapshotHandle make_snapshot()
{
    std::lock_guard lck{snapshot_mutex};
    return SnapshotHandle(new (snapshot_allocator.acquire()) EventRecordSnapshot);
}

void delete_snapshot(SnapshotHandle handle)
{
    auto *p = static_cast<EventRecordSnapshot *>(handle.id_);
    debug::assert(p);
    p->~EventRecordSnapshot();
    std::lock_guard lck{snapshot_mutex};
    snapshot_allocator.release(p);
}

EventRecordSnapshot &get_snapshot(SnapshotHandle handle)
{
    auto *p = static_cast<EventRecordSnapshot *>(handle.id_);
    debug::assert(p);
    return *p;
}
} // namespace aegis::sensor::win32::etw