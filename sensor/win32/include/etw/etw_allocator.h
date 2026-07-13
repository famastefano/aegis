#pragma once

#include <memory>

namespace aegis::sensor::win32::etw
{
struct EventRecordSnapshot;
void delete_snapshot(EventRecordSnapshot *p);

struct SnapshotDeleter
{
    void operator()(EventRecordSnapshot *p)
    { delete_snapshot(p); }
};

std::unique_ptr<EventRecordSnapshot, SnapshotDeleter> make_snapshot();
} // namespace aegis::sensor::win32::etw