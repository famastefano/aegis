cmake_minimum_required(VERSION 4.0)

project(
    Aegis
    VERSION 0.1.0
    LANGUAGES CXX
)

# Build Presets
add_library(aegis_build_default INTERFACE)
add_library(aegis::build_default ALIAS aegis_build_default)
target_compile_features(aegis_build_default INTERFACE cxx_std_23)
if(WIN32)
    target_compile_definitions(aegis_build_default INTERFACE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    target_compile_options(aegis_build_default INTERFACE
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
else()
    message(FATAL_ERROR "Linux TBD")
endif()

# 3rd-Party Deps
add_subdirectory(thirdparty/concurrentqueue)
add_subdirectory(thirdparty/googletest)

include(CTest)

# Generic code usable by any aegis module
add_subdirectory(common)

# Project modules
add_subdirectory(sensor)