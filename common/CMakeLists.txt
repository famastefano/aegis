add_library(aegis_common INTERFACE)
add_library(aegis::common ALIAS aegis_common)
target_include_directories(aegis_common INTERFACE include)
target_link_libraries(aegis_common INTERFACE aegis::build_default)

add_subdirectory(test)