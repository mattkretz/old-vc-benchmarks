cmake_minimum_required(VERSION 2.8.1)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BUILD_DIR)
   message(ERROR "You don't want to configure in the source directory!")
endif()

project(Vc-Benchmarks)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include (VcMacros)
include (AddTargetProperty)
include (OptimizeForArchitecture)

find_package(Qt4)

vc_determine_compiler()

set(disabled_targets)
if(VC_COMPILER_IS_GCC AND VC_GCC_VERSION STREQUAL "4.6.0")
   list(APPEND disabled_targets
      dhryrock_avx_benchmark
      )
endif()

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

vc_set_preferred_compiler_flags()

OptimizeForArchitecture()

if(CMAKE_BUILD_TYPE STREQUAL "" AND NOT CMAKE_CXX_FLAGS MATCHES "-O[123]")
   message(STATUS "WARNING! It seems you are compiling without optimization. Please set CMAKE_BUILD_TYPE.")
endif(CMAKE_BUILD_TYPE STREQUAL "" AND NOT CMAKE_CXX_FLAGS MATCHES "-O[123]")

add_custom_target(other VERBATIM)
add_custom_target(Scalar COMMENT "build Scalar code" VERBATIM)
add_custom_target(SSE COMMENT "build SSE code" VERBATIM)
add_custom_target(AVX COMMENT "build AVX code" VERBATIM)

set(BUILD_BENCHMARKS TRUE CACHE BOOL "Build benchmarks.")
if(BUILD_BENCHMARKS)
   add_subdirectory(benchmarks)
endif(BUILD_BENCHMARKS)
