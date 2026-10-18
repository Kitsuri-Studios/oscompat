cmake_minimum_required(VERSION 3.20)

project(wrapdl
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Cross-platform dynamic library loader wrapper"
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(BUILD_SHARED_LIB MATCHES "true")
    set(WRAPDL_BUILD_SHARED ON CACHE BOOL "" FORCE)
endif()
if(WRAPDL_DEBUG_BUILD MATCHES "true")
    set(WRAPDL_DEBUG ON CACHE BOOL "" FORCE)
endif()

option(WRAPDL_BUILD_SHARED "Build shared library instead of static" OFF)
option(WRAPDL_OPT_SIZE "Size-optimized build (Android-style -Os)" ON)
option(WRAPDL_DEBUG "Enable debug logging" OFF)

set(WRAPDL_SOURCES
    src/wrapdl.c
)

set(WRAPDL_HEADERS
    include/wrapdl.h
)

if(ANDROID)
    list(APPEND WRAPDL_SOURCES
        src/wdl_core.c
        src/wdl_iterate.c
        src/wdl_linker.c
        src/wdl_lzma.c
        src/wdl_util.c
        src/android/wdl_ns_bypass.c
        src/android/wdl_ns_patch.c
        src/android/wdl_ns_layout.c
        src/android/wdl_ns_runtime.c
    )
endif()

if(WRAPDL_BUILD_SHARED)
    add_library(wrapdl SHARED ${WRAPDL_SOURCES})
else()
    add_library(wrapdl STATIC ${WRAPDL_SOURCES})
endif()

add_library(wrapdl::wrapdl ALIAS wrapdl)

set_target_properties(wrapdl PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(wrapdl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(ANDROID)
    target_include_directories(wrapdl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/android)
endif()

target_compile_definitions(wrapdl PRIVATE WRAPDL_USING_BUILD_CONFIG)

if(WRAPDL_DEBUG)
    target_compile_definitions(wrapdl PRIVATE WRAPDL_DEBUG)
endif()

if(WRAPDL_BUILD_SHARED)
    target_compile_definitions(wrapdl PRIVATE WRAPDL_EXPORT_SYM)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT ANDROID)
    target_compile_definitions(wrapdl PRIVATE _GNU_SOURCE)
endif()

if(ANDROID)
    target_link_libraries(wrapdl PUBLIC dl log)

    if(WRAPDL_OPT_SIZE AND NOT USEASAN)
        target_compile_options(wrapdl PRIVATE -Os -ffunction-sections -fdata-sections)
        target_link_options(wrapdl PRIVATE -Os -Wl,--exclude-libs,ALL -Wl,--gc-sections)
        if(ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64")
            target_link_options(wrapdl PRIVATE "-Wl,-z,max-page-size=16384")
        endif()
    endif()
elseif(WIN32)
    target_link_libraries(wrapdl PUBLIC kernel32)
    if(WRAPDL_OPT_SIZE)
        target_compile_options(wrapdl PRIVATE -Os -ffunction-sections -fdata-sections)
    endif()
else()
    target_link_libraries(wrapdl PUBLIC ${CMAKE_DL_LIBS})
    if(WRAPDL_OPT_SIZE)
        target_compile_options(wrapdl PRIVATE -Os -ffunction-sections -fdata-sections)
        target_link_options(wrapdl PRIVATE -Os -Wl,--gc-sections)
    endif()
endif()

install(
    TARGETS wrapdl
    EXPORT wrapdlTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES ${WRAPDL_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT wrapdlTargets
    FILE wrapdlTargets.cmake
    NAMESPACE wrapdl::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wrapdl
)

configure_package_config_file(
    cmake/wrapdlConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/wrapdlConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wrapdl
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/wrapdlConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/wrapdlConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/wrapdlConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wrapdl
)

install(
    FILES LICENSE
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/wrapdl
)
