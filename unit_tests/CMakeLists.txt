cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# project information
project(unit_tests
        VERSION 0.1
	      DESCRIPTION "Unit tests for Ledger Nano application"
        LANGUAGES C)


# guard against bad build-type strings
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CTest)
ENABLE_TESTING()

# specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -g -O0 --coverage")

set(GCC_COVERAGE_LINK_FLAGS "--coverage -lgcov")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

add_compile_definitions(TEST)

include_directories(../src)
include_directories(../src/common)
include_directories(./mocks)

add_executable(test_numberHelpers tests/testNumberHelpers.c)
add_executable(test_time tests/testTime.c)
add_executable(test_base58Check tests/testBase58Check.c)
add_executable(test_util tests/testUtil.c)

add_library(numberHelpers ../src/common/numberHelpers.c)
add_library(time  ../src/common/numberHelpers.c ../src/common/time.c)
add_library(base58Check ../src/common/base58check.c mocks/shaMock.c mocks/sha256.c mocks/sha256.h)
add_library(util mocks/os.c mocks/mocks.c mocks/globals.c ../src/common/util.c)

target_link_libraries(test_numberHelpers PUBLIC cmocka numberHelpers)
target_link_libraries(test_time PUBLIC cmocka  time)
target_link_libraries(base58Check PUBLIC "-Wl,--wrap=cx_hash_sha256")
target_link_libraries(test_base58Check PUBLIC cmocka base58Check)
target_link_libraries(util PUBLIC numberHelpers base58Check)
target_link_libraries(test_util PUBLIC cmocka util)

add_test(test_numberHelpers test_numberHelpers)
add_test(test_time test_time)
add_test(test_base58Check test_base58Check)
add_test(test_util test_util)
