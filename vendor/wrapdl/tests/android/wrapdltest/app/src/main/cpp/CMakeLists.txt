cmake_minimum_required(VERSION 3.16)
project(wrapdltest)

get_filename_component(WRAPDL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../.." ABSOLUTE)

add_subdirectory("${WRAPDL_ROOT}" "${CMAKE_BINARY_DIR}/wrapdl")

add_library(dummyA SHARED dummies/dummyA.c)
add_library(dummyB SHARED dummies/dummyB.c)
add_library(dummyC SHARED dummies/dummyC.c)
target_include_directories(dummyC PRIVATE "${WRAPDL_ROOT}/include")
target_link_libraries(dummyC PRIVATE dummyB wrapdl)

if(ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64")
    target_link_options(dummyA PRIVATE "-Wl,-z,max-page-size=16384")
    target_link_options(dummyB PRIVATE "-Wl,-z,max-page-size=16384")
    target_link_options(dummyC PRIVATE "-Wl,-z,max-page-size=16384")
endif()

add_library(wrapdltest SHARED
    jni_bridge.cpp
    test_runner.cpp
    "${WRAPDL_ROOT}/tests/common/test_util.c"
)

target_include_directories(wrapdltest PRIVATE
    "${WRAPDL_ROOT}/include"
    "${WRAPDL_ROOT}/tests/common"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(wrapdltest PRIVATE wrapdl log android)
add_dependencies(wrapdltest dummyA dummyB dummyC)

target_compile_options(wrapdltest PRIVATE -Os -ffunction-sections -fdata-sections)
target_link_options(wrapdltest PRIVATE -Os -Wl,--gc-sections)

if(ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64")
    target_link_options(wrapdltest PRIVATE "-Wl,-z,max-page-size=16384")
endif()
