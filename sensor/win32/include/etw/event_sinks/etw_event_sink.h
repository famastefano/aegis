#pragma once

#include <etw/allocators/handle_fwd.h>

namespace aegis::sensor::win32::etw {

class IEtwEventSink {
public:
    virtual ~IEtwEventSink() = default;

    virtual void consume_event(SnapshotHandle handle) noexcept = 0;
};

} // namespace aegis::sensor::win32::etw
