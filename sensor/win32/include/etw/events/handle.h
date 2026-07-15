#pragma once

namespace aegis::sensor::win32::etw
{
struct SnapshotHandle
{
    void *id_{};
};

struct ExtendedDataHandle
{
    void *id_{};
};
} // namespace aegis::sensor::win32::etw