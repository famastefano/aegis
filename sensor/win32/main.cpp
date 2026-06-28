#include <etw/etw_session.h>
#include <etw/provider_registry.h>
#include <etw/self_test_provider.h>
#include <etw/stdout_event_sink.h>

#include <windows.h>

#include <atomic>
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
    using aegis::sensor::win32::etw::ProviderSelection;
    using aegis::sensor::win32::etw::SelfTestProvider;
    using aegis::sensor::win32::etw::StdoutEventSink;

    std::atomic_bool stop_requested{false};

    BOOL WINAPI console_control_handler(DWORD control_type) noexcept
    {
        switch (control_type)
        {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            stop_requested.store(true);
            return TRUE;
        default:
            return FALSE;
        }
    }

    struct ProgramOptions
    {
        unsigned int duration_seconds{10};
        bool enable_kernel_process{false};
        bool show_help{false};
    };

    void print_usage(std::ostream &output)
    {
        output << "Usage: aegis_sensor_win32 [--duration-seconds <seconds>] [--enable-kernel-process]\n"
               << '\n'
               << "Options:\n"
               << "  --duration-seconds <seconds>  Runtime duration. Use 0 to run until Ctrl+C.\n"
               << "  --enable-kernel-process       Enable Microsoft-Windows-Kernel-Process events.\n"
               << "  --help                        Show this help text.\n";
    }

    [[nodiscard]] unsigned int parse_duration(std::string_view value)
    {
        std::string text{value};
        std::size_t consumed{};
        const auto parsed = std::stoul(text, &consumed, 10);
        if (consumed != text.size())
        {
            throw std::invalid_argument{"duration contains non-numeric characters"};
        }
        if (parsed > static_cast<unsigned long>((std::numeric_limits<unsigned int>::max)()))
        {
            throw std::out_of_range{"duration is too large"};
        }
        return static_cast<unsigned int>(parsed);
    }

    [[nodiscard]] ProgramOptions parse_arguments(int argc, char **argv)
    {
        ProgramOptions options{};

        for (int index = 1; index < argc; ++index)
        {
            const std::string_view arg{argv[index]};
            if (arg == "--help" || arg == "-h")
            {
                options.show_help = true;
                continue;
            }
            if (arg == "--enable-kernel-process")
            {
                options.enable_kernel_process = true;
                continue;
            }
            if (arg == "--duration-seconds")
            {
                if ((index + 1) >= argc)
                {
                    throw std::invalid_argument{"--duration-seconds requires a value"};
                }
                options.duration_seconds = parse_duration(argv[++index]);
                continue;
            }

            throw std::invalid_argument{"unknown argument: " + std::string{arg}};
        }

        return options;
    }

    [[nodiscard]] std::wstring make_session_name()
    {
        return L"AegisSensorWin32-" + std::to_wstring(GetCurrentProcessId());
    }

    void run_self_test_loop(
        const std::optional<SelfTestProvider> &self_test_provider,
        unsigned int duration_seconds)
    {
        const auto start = std::chrono::steady_clock::now();
        std::uint32_t sequence{};

        while (!stop_requested.load())
        {
            if (self_test_provider.has_value())
            {
                static_cast<void>(self_test_provider->write_event(sequence++));
            }

            if (duration_seconds != 0)
            {
                const auto elapsed = std::chrono::steady_clock::now() - start;
                if (elapsed >= std::chrono::seconds{duration_seconds})
                {
                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds{250});
        }
    }

} // namespace

int main(int argc, char **argv)
{
    try
    {
        const auto options = parse_arguments(argc, argv);
        if (options.show_help)
        {
            print_usage(std::cout);
            return 0;
        }

        SetConsoleCtrlHandler(&console_control_handler, TRUE);

        std::optional<SelfTestProvider> self_test_provider;
        self_test_provider.emplace();

        StdoutEventSink sink{std::cout};
        ProviderSelection provider_selection{};
        provider_selection.enable_self_test = true;
        provider_selection.enable_kernel_process = options.enable_kernel_process;

        EtwSession session{
            EtwSession::Config{
                .session_name = make_session_name(),
                .providers = EtwProviderRegistry::build(provider_selection),
            },
            sink,
        };

        session.start();
        run_self_test_loop(self_test_provider, options.duration_seconds);
        session.stop();
        session.rethrow_consumer_error();

        SetConsoleCtrlHandler(&console_control_handler, FALSE);
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
