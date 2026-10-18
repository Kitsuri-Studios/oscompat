# Install oscompat, then build and run a real consumer project against the
# installed package through find_package. Driven by ctest from the top level.

if(NOT CONFIG)
	set(CONFIG Release)
endif()

execute_process(
	COMMAND "${CMAKE_COMMAND}" --install "${OSCOMPAT_BINARY_DIR}"
		--prefix "${USAGE_PREFIX}" --config "${CONFIG}"
	RESULT_VARIABLE result)
if(NOT result EQUAL 0)
	message(FATAL_ERROR "oscompat install failed (${result})")
endif()

file(REMOVE_RECURSE "${USAGE_BUILD_DIR}")

set(configureArgs
	-S "${USAGE_SOURCE_DIR}"
	-B "${USAGE_BUILD_DIR}"
	"-DCMAKE_PREFIX_PATH=${USAGE_PREFIX}"
	"-DCMAKE_BUILD_TYPE=${CONFIG}")
if(OSCOMPAT_C_FLAGS)
	list(APPEND configureArgs "-DCMAKE_C_FLAGS=${OSCOMPAT_C_FLAGS}")
endif()

execute_process(
	COMMAND "${CMAKE_COMMAND}" ${configureArgs}
	RESULT_VARIABLE result)
if(NOT result EQUAL 0)
	message(FATAL_ERROR "consumer configure failed (${result})")
endif()

execute_process(
	COMMAND "${CMAKE_COMMAND}" --build "${USAGE_BUILD_DIR}" --config "${CONFIG}"
	RESULT_VARIABLE result)
if(NOT result EQUAL 0)
	message(FATAL_ERROR "consumer build failed (${result})")
endif()

execute_process(
	COMMAND "${CTEST_COMMAND}" --test-dir "${USAGE_BUILD_DIR}"
		-C "${CONFIG}" --output-on-failure
	RESULT_VARIABLE result)
if(NOT result EQUAL 0)
	message(FATAL_ERROR "consumer test failed (${result})")
endif()
