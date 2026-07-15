#pragma once

#include <etw/events/handle_fwd.h>

struct _EVENT_RECORD;

namespace aegis::sensor::win32::etw
{
SnapshotHandle make_snapshot(const _EVENT_RECORD &record);
EventSnapshot &resolve_snapshot(SnapshotHandle handle);
void           delete_snapshot(SnapshotHandle handle);
void           destroy_factory();
} // namespace aegis::sensor::win32::etw