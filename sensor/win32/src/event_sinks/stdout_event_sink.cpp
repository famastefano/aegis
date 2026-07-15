#include <etw/allocators/event_record_allocator.h>
#include <etw/allocators/handle.h>
#include <etw/event_record_snapshot.h>
#include <etw/event_sinks/stdout_event_sink.h>

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

void StdoutEventSink::consume_event(SnapshotHandle handle) noexcept
{
    try
    {
        auto           &ev = get_snapshot(handle);
        std::lock_guard lock{output_mutex_};
        (*output_) << "provider=" << ev.header.provider_id << " pid=" << ev.header.process_id
                   << " tid=" << ev.header.thread_id
                   << " level=" << static_cast<unsigned int>(ev.header.ev_descriptor.Level)
                   << " opcode=" << static_cast<unsigned int>(ev.header.ev_descriptor.Opcode)
                   << " event_id=" << ev.header.ev_descriptor.Id
                   << " version=" << static_cast<unsigned int>(ev.header.ev_descriptor.Version) << '\n';
    }
    catch (...)
    {
    }
    delete_snapshot(handle);
}

} // namespace aegis::sensor::win32::etw
