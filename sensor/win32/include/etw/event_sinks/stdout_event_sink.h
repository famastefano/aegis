#pragma once

#include <etw/event_sinks/etw_event_sink.h>

#include <iosfwd>
#include <mutex>

namespace aegis::sensor::win32::etw {

class StdoutEventSink final : public IEtwEventSink {
public:
    explicit StdoutEventSink(std::ostream& output) noexcept;

    void consume_event(SnapshotHandle handle) noexcept override;

private:
    std::ostream* output_;
    std::mutex output_mutex_;
};

} // namespace aegis::sensor::win32::etw
