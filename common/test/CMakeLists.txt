if(NOT BUILD_TESTING)
    return()
endif()

add_executable(aegis_common_pool_allocator_tests
    pool_allocator_test.cpp
)

target_link_libraries(aegis_common_pool_allocator_tests
    PRIVATE
        aegis::common
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(aegis_common_pool_allocator_tests)
