add_library(aegis_sensor_win32_common INTERFACE)
target_compile_definitions(aegis_sensor_win32_common INTERFACE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)
target_compile_features(aegis_sensor_win32_common INTERFACE cxx_std_23)
target_compile_options(aegis_sensor_win32_common INTERFACE
    # C++ Conformance
    /permissive-
    /Zc:preprocessor
    /Zc:__cplusplus
    /Zc:enumTypes
    /volatile:iso
    /EHsc

    # Debugging
    $<$<CONFIG:Debug>:/Zi>
    /diagnostics:caret
    /W4

    # Hardening
    $<$<CONFIG:Debug>:/RTCsu>
    $<$<CONFIG:Release>:/guard:cf>
)

add_library(aegis_sensor_win32_core STATIC
    src/etw_session.cpp
    src/event_record_snapshot.cpp
    src/provider_registry.cpp
    src/self_test_provider.cpp
    src/stdout_event_sink.cpp
)
target_include_directories(aegis_sensor_win32_core PUBLIC include)
target_link_libraries(aegis_sensor_win32_core
    PUBLIC
        aegis_sensor_win32_common
    PRIVATE
        advapi32
)

add_executable(aegis_sensor_win32 main.cpp)
target_link_libraries(aegis_sensor_win32 PRIVATE aegis_sensor_win32_core)