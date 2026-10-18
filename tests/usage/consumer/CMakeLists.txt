cmake_minimum_required(VERSION 3.16)
project(oscompat_consumer LANGUAGES C)

find_package(oscompat CONFIG REQUIRED)

enable_testing()

add_executable(oscompat_consumer main.c)
target_link_libraries(oscompat_consumer PRIVATE oscompat::oscompat)
add_test(NAME consumer COMMAND oscompat_consumer)
