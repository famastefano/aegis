#include <etw/etw_allocator.h>
#include <etw/event_record_snapshot.h>

#include <aegis/allocators/pool_allocator.h>
#include <mutex>

namespace aegis::sensor::win32::etw
{
namespace
{
allocators::UnorderedPoolAllocator<EventRecordSnapshot> snapshot_allocator{256, 4};
std::mutex                                              snapshot_mutex;
} // namespace

snapshot_ptr_t make_snapshot()
{
    std::lock_guard lck{snapshot_mutex};
    return snapshot_ptr_t(new (snapshot_allocator.acquire()) EventRecordSnapshot);
}

void delete_snapshot(EventRecordSnapshot *p)
{
    p->~EventRecordSnapshot();
    std::lock_guard lck{snapshot_mutex};
    snapshot_allocator.release(p);
}
} // namespace aegis::sensor::win32::etw