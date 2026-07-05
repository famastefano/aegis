if(NOT BUILD_TESTING)
    return()
endif()

add_executable(aegis_common_pool_allocator_tests
    "pool_allocator_test.cpp"
    "pool_allocator_test_utils.h"
)
target_include_directories(aegis_common_pool_allocator_tests PRIVATE .)
target_link_libraries(aegis_common_pool_allocator_tests
    PRIVATE
        aegis::common
        gtest_main
)

if(AEGIS_STRESS_TEST)
    add_executable(aegis_common_pool_allocator_stress_tests
     "pool_allocator_stress_test.cpp"
     "pool_allocator_test_utils.h"
    )
    target_include_directories(aegis_common_pool_allocator_stress_tests PRIVATE .)
    target_link_libraries(aegis_common_pool_allocator_stress_tests
        PRIVATE
            aegis::common
            gtest_main
    )
endif()

include(GoogleTest)
gtest_discover_tests(aegis_common_pool_allocator_tests)
