add_library(aegis_sensor_win32_core STATIC
    src/etw_session.cpp
    src/provider_registry.cpp
    src/events/extended_data.cpp
    src/events/snapshot_factory.cpp
    src/event_sinks/stdout_event_sink.cpp
    src/event_sinks/pool_event_sink.cpp
)
target_include_directories(aegis_sensor_win32_core PUBLIC include)
target_link_libraries(aegis_sensor_win32_core
    PUBLIC
        aegis::build_default
    PRIVATE
        aegis::common
        thirdparty::concurrentqueue
        advapi32
        Tdh
)

add_executable(aegis_sensor_win32 main.cpp)
target_link_libraries(aegis_sensor_win32 PRIVATE aegis_sensor_win32_core aegis::common)
target_link_options(aegis_sensor_win32 PRIVATE
    "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'"
)