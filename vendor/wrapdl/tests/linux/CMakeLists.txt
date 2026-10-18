cmake_minimum_required(VERSION 3.20)
project(wrapdl_test_linux C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

get_filename_component(WRAPDL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
add_subdirectory("${WRAPDL_ROOT}" "${CMAKE_BINARY_DIR}/wrapdl")

add_library(dummyA SHARED dummies/dummyA.c)
add_library(dummyB SHARED dummies/dummyB.c)
add_library(dummyC SHARED dummies/dummyC.c)
target_include_directories(dummyC PRIVATE "${WRAPDL_ROOT}/include")
target_link_libraries(dummyC PRIVATE dummyB wrapdl::wrapdl)

add_executable(wrapdl_test
    test_main.c
    ../common/test_util.c
)
target_include_directories(wrapdl_test PRIVATE ../common "${WRAPDL_ROOT}/include")
target_compile_definitions(wrapdl_test PRIVATE "TEST_LIB_DIR=\"${CMAKE_BINARY_DIR}\"")
target_link_libraries(wrapdl_test PRIVATE wrapdl::wrapdl ${CMAKE_DL_LIBS})
add_dependencies(wrapdl_test dummyA dummyB dummyC)
target_compile_options(wrapdl_test PRIVATE -Os)
