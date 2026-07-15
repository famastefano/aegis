#pragma once

#include <etw/allocators/handle_fwd.h>

namespace aegis::sensor::win32::etw
{
SnapshotHandle make_snapshot();
void delete_snapshot(SnapshotHandle handle);
EventRecordSnapshot& get_snapshot(SnapshotHandle handle);
} // namespace aegis::sensor::win32::etw