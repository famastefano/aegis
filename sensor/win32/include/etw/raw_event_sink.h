#pragma once

#include <etw/event_record_snapshot.h>

namespace aegis::sensor::win32::etw {

class IRawEventSink {
public:
    virtual ~IRawEventSink() = default;

    virtual void on_event(const EventRecordSnapshot& event) noexcept = 0;
};

} // namespace aegis::sensor::win32::etw
