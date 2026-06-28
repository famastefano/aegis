#include <etw/stdout_event_sink.h>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace aegis::sensor::win32::etw {
namespace {

[[nodiscard]] std::string format_guid(const GUID& guid)
{
    std::ostringstream output;
    output << std::hex << std::setfill('0') << std::nouppercase;
    output << '{' << std::setw(8) << guid.Data1 << '-';
    output << std::setw(4) << guid.Data2 << '-';
    output << std::setw(4) << guid.Data3 << '-';
    output << std::setw(2) << static_cast<unsigned int>(guid.Data4[0]);
    output << std::setw(2) << static_cast<unsigned int>(guid.Data4[1]) << '-';
    for (auto index = 2; index < 8; ++index) {
        output << std::setw(2) << static_cast<unsigned int>(guid.Data4[index]);
    }
    output << '}';
    return output.str();
}

} // namespace

StdoutEventSink::StdoutEventSink(std::ostream& output) noexcept
    : output_(&output)
{
}

void StdoutEventSink::on_event(const EventRecordSnapshot& event) noexcept
{
    try {
        std::lock_guard lock{output_mutex_};
        (*output_) << "provider=" << format_guid(event.provider_id)
                   << " pid=" << event.process_id
                   << " tid=" << event.thread_id
                   << " level=" << static_cast<unsigned int>(event.level)
                   << " opcode=" << static_cast<unsigned int>(event.opcode)
                   << " event_id=" << event.event_id
                   << " version=" << static_cast<unsigned int>(event.version)
                   << '\n';
    } catch (...) {
    }
}

} // namespace aegis::sensor::win32::etw
