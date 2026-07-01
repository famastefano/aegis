cmake_minimum_required(VERSION 4.0)

project(
    Aegis
    VERSION 0.1.0
    LANGUAGES CXX
)

add_subdirectory(thirdparty/concurrentqueue)
add_subdirectory(thirdparty/googletest)

add_subdirectory(sensor)