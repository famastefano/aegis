#pragma once

#include <etw/event_sinks/etw_event_sink.h>

namespace aegis::sensor::win32::etw
{
class PoolEventSink : public IEtwEventSink
{
  public:
    ~PoolEventSink() override;
    void consume_event(SnapshotHandle handle) noexcept override;
};
} // namespace aegis::sensor::win32::etw