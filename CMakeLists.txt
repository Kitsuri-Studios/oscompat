cmake_minimum_required(VERSION 3.16)
project(oscompat VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(OSCOMPAT_BUILD_TESTS "Build the test suite" ON)

file(GLOB_RECURSE OSCOMPAT_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set(OSCOMPAT_WRAPDL_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/wrapdl.c")
if(ANDROID)
	list(APPEND OSCOMPAT_WRAPDL_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/wdl_core.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/wdl_iterate.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/wdl_linker.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/wdl_lzma.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/wdl_util.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/android/wdl_ns_bypass.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/android/wdl_ns_patch.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/android/wdl_ns_layout.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/android/wdl_ns_runtime.c")
endif()

add_library(oscompat_wrapdl OBJECT ${OSCOMPAT_WRAPDL_SOURCES})
set_target_properties(oscompat_wrapdl PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE ON)
target_include_directories(oscompat_wrapdl PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src")
if(ANDROID)
	target_include_directories(oscompat_wrapdl PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/src/android")
endif()

add_library(oscompat STATIC ${OSCOMPAT_SOURCES})
target_sources(oscompat PRIVATE $<TARGET_OBJECTS:oscompat_wrapdl>)
add_library(oscompat::oscompat ALIAS oscompat)

target_include_directories(oscompat
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/wrapdl/include")
set_target_properties(oscompat PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(WIN32)
	target_compile_definitions(oscompat PRIVATE _CRT_SECURE_NO_WARNINGS)
	target_compile_definitions(oscompat_wrapdl PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
	target_compile_definitions(oscompat PRIVATE _GNU_SOURCE _DEFAULT_SOURCE)
	target_compile_definitions(oscompat_wrapdl PRIVATE _GNU_SOURCE _DEFAULT_SOURCE)
endif()

if(CMAKE_DL_LIBS)
	target_link_libraries(oscompat PUBLIC ${CMAKE_DL_LIBS})
endif()

if(ANDROID)
	target_link_libraries(oscompat PUBLIC log)
endif()

if(MSVC)
	set(OSCOMPAT_WARNINGS /W4)
else()
	set(OSCOMPAT_WARNINGS -Wall -Wextra)
endif()
target_compile_options(oscompat PRIVATE ${OSCOMPAT_WARNINGS})
target_compile_options(oscompat_wrapdl PRIVATE ${OSCOMPAT_WARNINGS})

if(OSCOMPAT_BUILD_TESTS)
	enable_testing()

	add_library(oscompat_usage_plugin MODULE
		"${CMAKE_CURRENT_SOURCE_DIR}/tests/usage/plugin/usage_plugin.c")
	set_target_properties(oscompat_usage_plugin PROPERTIES PREFIX "")
	target_compile_options(oscompat_usage_plugin PRIVATE ${OSCOMPAT_WARNINGS})

	file(GLOB_RECURSE OSCOMPAT_TEST_SOURCES CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/tests/*/test_*.c")
	foreach(oscompatTestSource ${OSCOMPAT_TEST_SOURCES})
		get_filename_component(oscompatTestName ${oscompatTestSource} NAME_WE)
		add_executable(oscompat_${oscompatTestName} ${oscompatTestSource})
		target_link_libraries(oscompat_${oscompatTestName} PRIVATE oscompat)
		target_include_directories(oscompat_${oscompatTestName} PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/tests)
		target_compile_options(oscompat_${oscompatTestName} PRIVATE ${OSCOMPAT_WARNINGS})
		if(oscompatTestName STREQUAL "test_usage_dynlib")
			add_dependencies(oscompat_${oscompatTestName} oscompat_usage_plugin)
			add_test(NAME ${oscompatTestName}
				COMMAND oscompat_${oscompatTestName} $<TARGET_FILE:oscompat_usage_plugin>)
		else()
			add_test(NAME ${oscompatTestName} COMMAND oscompat_${oscompatTestName})
		endif()
	endforeach()

	if(NOT CMAKE_CROSSCOMPILING)
		add_test(NAME usage_package
			COMMAND ${CMAKE_COMMAND}
				"-DOSCOMPAT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
				"-DOSCOMPAT_C_FLAGS=${CMAKE_C_FLAGS}"
				"-DUSAGE_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/tests/usage/consumer"
				"-DUSAGE_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/usage-prefix"
				"-DUSAGE_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}/usage-consumer"
				"-DCONFIG=$<CONFIG>"
				"-DCTEST_COMMAND=${CMAKE_CTEST_COMMAND}"
				-P "${CMAKE_CURRENT_SOURCE_DIR}/tests/usage/package_test.cmake")
	endif()
endif()

install(TARGETS oscompat EXPORT oscompatTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT oscompatTargets
	NAMESPACE oscompat::
	FILE oscompatTargets.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/oscompat)

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/oscompatConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/oscompatConfig.cmake"
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/oscompat)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/oscompatConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion)
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/oscompatConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/oscompatConfigVersion.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/oscompat)

set(OSCOMPAT_PC_LIBS "")
foreach(oscompatDlLib ${CMAKE_DL_LIBS})
	set(OSCOMPAT_PC_LIBS "${OSCOMPAT_PC_LIBS} -l${oscompatDlLib}")
endforeach()
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/oscompat.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/oscompat.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/oscompat.pc"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
