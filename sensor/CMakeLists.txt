cmake_minimum_required(VERSION 4.0)

if(WIN32 AND MSVC)
    add_subdirectory(win32)
endif()