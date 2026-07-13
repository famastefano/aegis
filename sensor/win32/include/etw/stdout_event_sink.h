#pragma once

#include <etw/raw_event_sink.h>

#include <iosfwd>
#include <mutex>

namespace aegis::sensor::win32::etw {

class StdoutEventSink final : public IRawEventSink {
public:
    explicit StdoutEventSink(std::ostream& output) noexcept;

    void consume_event(snapshot_ptr_t&& event) noexcept override;

private:
    std::ostream* output_;
    std::mutex output_mutex_;
};

} // namespace aegis::sensor::win32::etw
