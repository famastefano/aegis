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

add_executable(aegis_sensor_win32 main.cpp)
target_link_libraries(aegis_sensor_win32 PRIVATE aegis_sensor_win32_common)
