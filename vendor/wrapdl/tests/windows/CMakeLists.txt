cmake_minimum_required(VERSION 3.20)
project(wrapdl_test_win C)

set(CMAKE_C_STANDARD 11)

get_filename_component(WRAPDL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
add_subdirectory("${WRAPDL_ROOT}" "${CMAKE_BINARY_DIR}/wrapdl")

add_executable(wrapdl_test
    test_main.c
    ../common/test_util.c
)
target_include_directories(wrapdl_test PRIVATE ../common "${WRAPDL_ROOT}/include")
target_link_libraries(wrapdl_test PRIVATE wrapdl::wrapdl)
if(MSVC)
    target_compile_options(wrapdl_test PRIVATE /O1)
else()
    target_compile_options(wrapdl_test PRIVATE -Os)
endif()
