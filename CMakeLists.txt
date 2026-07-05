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

if(BUILD_TESTING)
    target_compile_definitions(aegis_build_default INTERFACE AEGIS_BUILD_TEST)
endif()

if(AEGIS_STRESS_TEST)
    target_compile_definitions(aegis_build_default INTERFACE AEGIS_BUILD_STRESS_TEST)
endif()

if(WIN32)
    target_compile_definitions(aegis_build_default INTERFACE
        AEGIS_TARGET_WINDOWS
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
    target_compile_definitions(aegis_build_default INTERFACE
        AEGIS_TARGET_LINUX
        $<$<CONFIG:Debug>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG>
        $<$<CONFIG:Release>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE>
    )
    target_compile_options(aegis_build_default INTERFACE
        # C++ Conformance
        -pedantic-errors
        -Wundefined-internal-type
        -Wundefined-reinterpret-cast
        -Wsometimes-uninitialized
        -Wnon-virtual-dtor

        # Debugging
        -Wall
        -Wextra
        -Wdeprecated
        -Wloop-analysis
        -Wmethod-signatures
        -Wthread-safety
        $<$<CONFIG:Debug>:-g -g3 -fno-omit-frame-pointer>

        # Hardening
        -Walloca

        # Performance
        -Wnrvo
        -Wweak-vtables
    )
    if(BUILD_TESTING)
        target_compile_options(aegis_build_default INTERFACE
            -fsanitize=address
            -fsanitize-address-use-after-scope
            -fsanitize-address-use-after-return=always
        )
        target_link_libraries(aegis_build_default INTERFACE
            -fsanitize=address
        )
    endif()
endif()

# 3rd-Party Deps
add_subdirectory(thirdparty/concurrentqueue)
add_subdirectory(thirdparty/googletest)

include(CTest)

# Generic code usable by any aegis module
add_subdirectory(common)

# Project modules
add_subdirectory(sensor)