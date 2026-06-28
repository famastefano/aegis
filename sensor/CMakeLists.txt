cmake_minimum_required(VERSION 4.0)

project(
    AegisSensor
    VERSION 0.1.0
    DESCRIPTION "Aegis endpoint telemetry sensor"
    LANGUAGES CXX
)

if(WIN32 AND MSVC)
    add_subdirectory(win32)
endif()