cmake_minimum_required(VERSION 3.12)

project(
  rp2040-emulator
  VERSION 1.0
  DESCRIPTION "A Raspberry Pi Pico Emulator in C++"
  LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

set(TARGET_APP rp2040-emulator)
add_executable(${TARGET_APP} ${SRC_FILES})
target_compile_options(${TARGET_APP} PUBLIC -Wall -Werror)
target_include_directories(${TARGET_APP} PUBLIC ${CMAKE_SOURCE_DIR}/src/include)
target_link_libraries(${TARGET_APP} -static)


message(STATUS "Fetching googletest...")
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        v1.13.0
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  set(gtest_build_tests OFF CACHE BOOL "Enable gtest tests" FORCE)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(TARGET_TEST run_rp2040_tests)
add_executable(${TARGET_TEST} ${LIB_FILES} ${TEST_FILES})
target_compile_options(${TARGET_TEST} PUBLIC -Wall -Werror)
target_include_directories(${TARGET_TEST} PUBLIC ${CMAKE_SOURCE_DIR}/src/include)
target_link_libraries(${TARGET_TEST} PRIVATE -static gtest gtest_main pthread)