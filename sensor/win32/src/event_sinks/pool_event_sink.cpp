#include <etw/event_sinks/pool_event_sink.h>
#include <etw/events/handle.h>

namespace aegis::sensor::win32::etw
{
PoolEventSink::~PoolEventSink()
{
}
void PoolEventSink::consume_event(SnapshotHandle handle) noexcept
{
    /*
     1. Push handle into worker queue
     2. Notify once
    */

    /*
     1. while queue has snapshots
        i. pull once from queue
       ii. convert into internal schema
      iii. send to schema serializer
       iv. push into free snapshot queue
     2. wait for snapshots
    */
}
} // namespace aegis::sensor::win32::etw