#include <etw/stdout_event_sink.h>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace aegis::sensor::win32::etw
{
namespace
{

[[nodiscard]] std::string format_guid(const GUID &guid)
{
    std::ostringstream output;
    output << std::hex << std::setfill('0') << std::nouppercase;
    output << '{' << std::setw(8) << guid.Data1 << '-';
    output << std::setw(4) << guid.Data2 << '-';
    output << std::setw(4) << guid.Data3 << '-';
    output << std::setw(2) << static_cast<unsigned int>(guid.Data4[0]);
    output << std::setw(2) << static_cast<unsigned int>(guid.Data4[1]) << '-';
    for (auto index = 2; index < 8; ++index)
    {
        output << std::setw(2) << static_cast<unsigned int>(guid.Data4[index]);
    }
    output << '}';
    return output.str();
}

} // namespace

StdoutEventSink::StdoutEventSink(std::ostream &output) noexcept : output_(&output)
{
}

void StdoutEventSink::consume_event(snapshot_ptr_t &&event) noexcept
{
    try
    {
        std::lock_guard lock{output_mutex_};
        (*output_) << "provider=" << event->header.provider_id << " pid=" << event->header.process_id
                   << " tid=" << event->header.thread_id
                   << " level=" << static_cast<unsigned int>(event->header.ev_descriptor.Level)
                   << " opcode=" << static_cast<unsigned int>(event->header.ev_descriptor.Opcode)
                   << " event_id=" << event->header.ev_descriptor.Id
                   << " version=" << static_cast<unsigned int>(event->header.ev_descriptor.Version) << '\n';
    }
    catch (...)
    {
    }
}

} // namespace aegis::sensor::win32::etw
