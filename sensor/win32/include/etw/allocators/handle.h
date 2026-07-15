#pragma once

namespace aegis::sensor::win32::etw
{
struct SnapshotHandle
{
    explicit SnapshotHandle(void *id) : id_(id)
    {
    }
    void *id_{};
};

struct ExtendedDataHandle
{
    explicit ExtendedDataHandle(void *id) : id_(id)
    {
    }
    void *id_{};
};
} // namespace aegis::sensor::win32::etw