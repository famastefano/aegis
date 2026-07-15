#pragma once

#include <Windows.h>

#include <evntrace.h>

#include <atomic>
#include <exception>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace aegis::sensor::win32::etw
{

class IEtwEventSink;

class EtwSession final
{
  public:
    struct Config
    {
        std::wstring               session_name;
        std::vector<std::uint16_t> providers;
    };

    EtwSession(Config config, IEtwEventSink &sink);
    ~EtwSession() noexcept;

    EtwSession(EtwSession const &)            = delete;
    EtwSession &operator=(EtwSession const &) = delete;
    EtwSession(EtwSession &&)                 = delete;
    EtwSession &operator=(EtwSession &&)      = delete;

    void start();
    void stop() noexcept;
    void rethrow_consumer_error() const;

  private:
    static void WINAPI event_record_callback(EVENT_RECORD *record) noexcept;

    void consume_trace(std::promise<void> ready) noexcept;
    void enable_providers() const;
    void handle_event(const EVENT_RECORD &record) noexcept;
    void set_consumer_error(std::exception_ptr error) noexcept;
    bool has_consumer_error() const noexcept;

    [[nodiscard]] static std::vector<unsigned char> make_trace_properties(std::wstring const &session_name);

    Config             config_;
    IEtwEventSink     &sink_;
    TRACEHANDLE        session_handle_{};
    std::jthread       consumer_thread_;
    mutable std::mutex error_mutex_;
    std::exception_ptr consumer_error_;
    std::atomic_bool   session_started_{false};
};

} // namespace aegis::sensor::win32::etw
