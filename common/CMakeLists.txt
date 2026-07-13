add_library(aegis_common STATIC
	src/debug/debug.cpp
)
add_library(aegis::common ALIAS aegis_common)
target_include_directories(aegis_common PUBLIC include "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(aegis_common PUBLIC aegis::build_default)
generate_export_header(aegis_common)

add_subdirectory(test)