add_library(aegis_sensor_win32_core STATIC
    src/events/extended_data.cpp
    src/etw_session.cpp
    src/event_record_snapshot.cpp
    src/provider_registry.cpp
    src/self_test_provider.cpp
    src/event_sinks/stdout_event_sink.cpp
    src/event_sinks/pool_event_sink.cpp
    src/allocators/event_record_allocator.cpp
)
target_include_directories(aegis_sensor_win32_core PUBLIC include)
target_link_libraries(aegis_sensor_win32_core
    PUBLIC
        aegis::build_default
    PRIVATE
        aegis::common
        thirdparty::concurrentqueue
        advapi32
)

add_executable(aegis_sensor_win32 main.cpp)
target_link_libraries(aegis_sensor_win32 PRIVATE aegis_sensor_win32_core)