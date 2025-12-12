cmake_minimum_required(VERSION 3.10)
project(StandaloneFuzzer VERSION 1.0 DESCRIPTION "Standalone Export Private Key Fuzzer" LANGUAGES C)

if (NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "Fuzzer needs to be built with Clang")
endif()

# Favor ClusterFuzzLite-provided flags if present; otherwise use safe defaults.
set(FALLBACK_CFLAGS "-O1 -g -fno-omit-frame-pointer -fsanitize-coverage=trace-cmp,indirect-calls,8bit-counters")
if(DEFINED ENV{SANITIZER})
  if("$ENV{SANITIZER}" STREQUAL "address")
    string(APPEND FALLBACK_CFLAGS " -fsanitize=address -fsanitize-address-use-after-scope")
  elseif("$ENV{SANITIZER}" STREQUAL "undefined")
    string(APPEND FALLBACK_CFLAGS " -fsanitize=undefined -fno-sanitize-recover=undefined")
  elseif("$ENV{SANITIZER}" STREQUAL "memory")
    string(APPEND FALLBACK_CFLAGS " -fsanitize=memory -fsanitize-memory-track-origins=2")
  endif()
endif()

# compatible with ClusterFuzzLite
if (NOT DEFINED ENV{LIB_FUZZING_ENGINE})
	set(COMPILATION_FLAGS_ "-g -Wall -fsanitize=fuzzer,address,undefined")
else()
	set(COMPILATION_FLAGS_ "$ENV{LIB_FUZZING_ENGINE} $ENV{CFLAGS}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

string(REPLACE " " ";" COMPILATION_FLAGS ${COMPILATION_FLAGS_})


add_executable(standalone_export_pk_new_path_fuzzer src/standalone_export_pk_new_path_fuzzer.c)
target_compile_options(standalone_export_pk_new_path_fuzzer PUBLIC ${COMPILATION_FLAGS})
target_link_options(standalone_export_pk_new_path_fuzzer PUBLIC ${COMPILATION_FLAGS})

add_executable(standalone_plt_fuzzer src/standalone_plt_fuzzer.c)
target_compile_options(standalone_plt_fuzzer PUBLIC ${COMPILATION_FLAGS})
target_link_options(standalone_plt_fuzzer PUBLIC ${COMPILATION_FLAGS})
