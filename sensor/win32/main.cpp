#include <etw/etw_session.h>
#include <etw/event_sinks/stdout_event_sink.h>
#include <etw/provider_registry.h>

#include <windows.h>

#include <atomic>
#include <charconv>
#include <chrono>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

namespace
{

using aegis::sensor::win32::etw::EtwProviderRegistry;
using aegis::sensor::win32::etw::EtwSession;
using aegis::sensor::win32::etw::StdoutEventSink;

std::atomic_bool stop_requested{false};

BOOL WINAPI console_control_handler(DWORD control_type) noexcept
{
    switch (control_type)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT: stop_requested.store(true); return TRUE;
    default: return FALSE;
    }
}

struct ProgramOptions
{
    unsigned int  duration_seconds{10};
    bool          show_help{false};
    bool          show_providers{false};
    std::uint16_t provider_id = USHRT_MAX;
};

void print_usage(std::ostream &output)
{
    output << "Usage: aegis_sensor_win32 [--help|-h] [--duration-seconds <secs>] [--show-providers] [--provider-id <id>]\n\n"
              "Options:\n"
              "  --help|-h                     Show this help text.\n"
              "  --duration-seconds <seconds>  Runtime duration. Use 0 to run until Ctrl+C.\n"
              "  --show-providers              Prints all the available providers found in the registry.\n"
              "  --provider-id <id>            Enables the provider with ID <id>.\n"
           << std::endl;
}

template <typename TNumber>
[[nodiscard]] void parse_number(TNumber &v, std::string_view value, TNumber range_min = std::numeric_limits<TNumber>::min(), TNumber range_max = std::numeric_limits<TNumber>::min(), bool clamp = false)
{
    if (auto ec = std::from_chars(value.data(), value.data() + value.size(), v).ec; static_cast<bool>(ec))
    {
        if (clamp || (range_min <= v && v <= range_max))
        {
            v = std::clamp(v, range_min, range_max);
            return;
        }
    }
    throw std::invalid_argument{"Invalid number."};
}

[[nodiscard]] ProgramOptions parse_arguments(int argc, char **argv)
{
    ProgramOptions options{};
    for (int index = 1; index < argc; ++index)
    {
        bool const has_next = (index + 1) < argc;

        std::string_view const arg{argv[index]};
        if (arg == "--help" || arg == "-h")
        {
            options.show_help = true;
        }
        else if (arg == "--show-providers")
        {
            options.show_providers = true;
        }
        else if (arg == "--duration-seconds")
        {
            if (!has_next)
                throw std::invalid_argument{"--duration-seconds requires a value"};

            parse_number(options.duration_seconds, argv[++index]);
        }
        else if (arg == "--provider-id")
        {
            if (!has_next)
                throw std::invalid_argument{"--provider-id requires a value"};

            parse_number(options.provider_id, argv[++index], std::uint16_t(0), std::uint16_t(USHRT_MAX - 1));
        }
        else
        {
            throw std::invalid_argument{"unknown argument: " + std::string{arg}};
        }
    }
    return options;
}

[[nodiscard]] std::wstring make_session_name()
{ return L"AegisSensorWin32-" + std::to_wstring(GetCurrentProcessId()); }

} // namespace

int main(int argc, char **argv)
{
    try
    {
        auto const options = parse_arguments(argc, argv);
        if (options.show_help)
        {
            print_usage(std::cout);
            return 0;
        }

        auto &registry = EtwProviderRegistry::get_registry();
        registry.discover_providers();

        if (options.show_providers)
        {
            std::size_t const count = registry.get_providers_count();
            for (std::size_t i = 0; i < count; ++i)
            {
                if (auto const info = registry.try_get_provider_info(i); info)
                    std::wcout << "#" << i << " id(" << info->id_ << "): " << info->name_ << "\n";
            }
            return 0;
        }

        if (options.provider_id == USHRT_MAX)
        {
            print_usage(std::cout);
            throw std::out_of_range("Invalid provider ID.");
        }

        SetConsoleCtrlHandler(&console_control_handler, TRUE);

        StdoutEventSink sink{std::cout};

        EtwSession session{
            EtwSession::Config{
                               .session_name = make_session_name(),
                               .providers    = {options.provider_id},
                               },
            sink,
        };

        session.start();
        session.stop();
        session.rethrow_consumer_error();

        SetConsoleCtrlHandler(&console_control_handler, FALSE);
        return 0;
    }
    catch (std::exception const &error)
    {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
