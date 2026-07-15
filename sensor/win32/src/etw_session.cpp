#include <etw/allocators/handle.h>
#include <etw/etw_session.h>
#include <etw/event_record_snapshot.h>
#include <etw/event_sinks/etw_event_sink.h>
#include <etw/provider_registry.h>

#include <evntcons.h>

#include <cstring>
#include <future>
#include <system_error>
#include <utility>

namespace aegis::sensor::win32::etw
{
namespace
{

[[noreturn]] void throw_win32_status(ULONG status, std::string const &message)
{ throw std::system_error{static_cast<int>(status), std::system_category(), message}; }

[[nodiscard]] bool is_stop_success(ULONG status) noexcept
{ return status == ERROR_SUCCESS || status == ERROR_WMI_INSTANCE_NOT_FOUND; }

} // namespace

EtwSession::EtwSession(Config config, IEtwEventSink &sink) : config_(std::move(config)), sink_(sink)
{
}

EtwSession::~EtwSession() noexcept
{ stop(); }

void EtwSession::start()
{
    if (session_started_.load())
    {
        return;
    }

    auto const &registry = EtwProviderRegistry::get_registry();
    for (std::uint16_t id : config_.providers)
    {
        if (!registry.try_get_provider_info_by_id(id))
            throw std::runtime_error{"Couldn't find provider."};
    }

    auto       properties   = make_trace_properties(config_.session_name);
    auto const start_status = StartTraceW(&session_handle_, config_.session_name.c_str(),
                                          reinterpret_cast<EVENT_TRACE_PROPERTIES *>(properties.data()));
    if (start_status != ERROR_SUCCESS)
    {
        throw_win32_status(start_status, "StartTraceW failed");
    }
    session_started_.store(true);

    try
    {
        enable_providers();

        std::promise<void> ready;
        auto               ready_future = ready.get_future();
        consumer_thread_                = std::jthread{
            [this, ready = std::move(ready)]() mutable { consume_trace(std::move(ready)); },
        };
        ready_future.get();
    }
    catch (...)
    {
        stop();
        throw;
    }
}

void EtwSession::stop() noexcept
{
    if (session_started_.exchange(false))
    {
        auto       properties = make_trace_properties(config_.session_name);
        auto const stop_status =
            ControlTraceW(session_handle_, config_.session_name.c_str(),
                          reinterpret_cast<EVENT_TRACE_PROPERTIES *>(properties.data()), EVENT_TRACE_CONTROL_STOP);
        if (!is_stop_success(stop_status)) {}
        session_handle_ = 0;
    }
}

void EtwSession::rethrow_consumer_error() const
{
    std::lock_guard lock{error_mutex_};
    if (consumer_error_)
    {
        std::rethrow_exception(consumer_error_);
    }
}

void WINAPI EtwSession::event_record_callback(EVENT_RECORD *record) noexcept
{
    if (record == nullptr || record->UserContext == nullptr)
    {
        return;
    }

    auto *session = static_cast<EtwSession *>(record->UserContext);
    session->handle_event(*record);
}

void EtwSession::consume_trace(std::promise<void> ready) noexcept
{
    EVENT_TRACE_LOGFILEW trace_log{};
    trace_log.LoggerName          = const_cast<LPWSTR>(config_.session_name.c_str());
    trace_log.ProcessTraceMode    = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    trace_log.EventRecordCallback = &EtwSession::event_record_callback;
    trace_log.Context             = this;

    auto trace_handle = OpenTraceW(&trace_log);
    if (trace_handle == INVALID_PROCESSTRACE_HANDLE)
    {
        ready.set_exception(std::make_exception_ptr(std::system_error{
            static_cast<int>(GetLastError()),
            std::system_category(),
            "OpenTraceW failed",
        }));
        return;
    }

    ready.set_value();

    auto const process_status = ProcessTrace(&trace_handle, 1, nullptr, nullptr);
    if (process_status != ERROR_SUCCESS && process_status != ERROR_CANCELLED)
    {
        set_consumer_error(std::make_exception_ptr(std::system_error{
            static_cast<int>(process_status),
            std::system_category(),
            "ProcessTrace failed",
        }));
    }

    if (auto const ret = CloseTrace(trace_handle); ret != ERROR_SUCCESS && ret != ERROR_CTX_CLOSE_PENDING)
    {
        // CloseTrace might fail if we already failed the trace for other reasons,
        // so leave the error as-is, as it might be more meaningful.
        if (!has_consumer_error())
        {
            set_consumer_error(std::make_exception_ptr(std::system_error{
                static_cast<int>(ret),
                std::system_category(),
                "CloseTrace failed",
            }));
        }
    }
}

void EtwSession::enable_providers() const
{
    auto const &registry = EtwProviderRegistry::get_registry();
    for (std::uint16_t id : config_.providers)
    {
        auto const info = registry.try_get_provider_info_by_id(id);

        ENABLE_TRACE_PARAMETERS parameters{};
        parameters.Version = ENABLE_TRACE_PARAMETERS_VERSION_2;

        auto const status = EnableTraceEx2(session_handle_, info->guid_, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_INFORMATION, 0, 0, 0, &parameters);
        if (status != ERROR_SUCCESS)
            throw_win32_status(status, "EnableTraceEx2 failed for provider");
    }
}

void EtwSession::handle_event(const EVENT_RECORD &record) noexcept
{
    try
    {
        sink_.consume_event(make_event_record_snapshot(record));
    }
    catch (...)
    {
        std::lock_guard lock{error_mutex_};
        consumer_error_ = std::current_exception();
    }
}

void EtwSession::set_consumer_error(std::exception_ptr error) noexcept
{
    try
    {
        std::lock_guard lock{error_mutex_};
        consumer_error_ = error;
    }
    catch (...)
    {
    }
}

bool EtwSession::has_consumer_error() const noexcept
{
    std::lock_guard lock{error_mutex_};
    return static_cast<bool>(consumer_error_);
}

std::vector<unsigned char> EtwSession::make_trace_properties(std::wstring const &session_name)
{
    auto const                 session_name_bytes = (session_name.size() + 1) * sizeof(wchar_t);
    std::vector<unsigned char> buffer(sizeof(EVENT_TRACE_PROPERTIES) + session_name_bytes);

    auto *properties                = reinterpret_cast<EVENT_TRACE_PROPERTIES *>(buffer.data());
    properties->Wnode.BufferSize    = static_cast<ULONG>(buffer.size());
    properties->Wnode.Flags         = WNODE_FLAG_TRACED_GUID;
    properties->Wnode.ClientContext = 1;
    properties->LogFileMode         = EVENT_TRACE_REAL_TIME_MODE;
    properties->BufferSize          = 64;
    properties->MinimumBuffers      = 4;
    properties->MaximumBuffers      = 16;
    properties->FlushTimer          = 1;
    properties->LoggerNameOffset    = sizeof(EVENT_TRACE_PROPERTIES);

    auto *name_destination = reinterpret_cast<wchar_t *>(buffer.data() + properties->LoggerNameOffset);
    std::memcpy(name_destination, session_name.c_str(), session_name_bytes);
    return buffer;
}

} // namespace aegis::sensor::win32::etw
