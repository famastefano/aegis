#include <etw/event_sinks/pool_event_sink.h>
#include <etw/allocators/handle.h>

namespace aegis::sensor::win32::etw
{
PoolEventSink::~PoolEventSink()
{
}
void PoolEventSink::consume_event(SnapshotHandle handle) noexcept
{
}
} // namespace aegis::sensor::win32::etw