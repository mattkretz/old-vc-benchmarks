# Locate the Vc build and source directories.
#
# Copyright 2009-2012   Matthias Kretz <kretz@kde.org>
#
# This module defines the following variables:
# Vc_FOUND
# Vc_INCLUDE_DIR
# Vc_LIBRARIES
# Vc_DEFINITIONS
# Vc_VERSION_MAJOR
# Vc_VERSION_MINOR
# Vc_VERSION_PATCH
# Vc_VERSION
# Vc_VERSION_STRING
# Vc_INSTALL_DIR
# Vc_LIB_DIR
# Vc_CMAKE_MODULES_DIR
#
# The following two variables are set according to the compiler used. Feel free
# to use them to skip whole compilation units.
# Vc_SSE_INTRINSICS_BROKEN
# Vc_AVX_INTRINSICS_BROKEN

set(Vc_FOUND false)
set(Vc_BINARY_DIR "$ENV{HOME}/obj/Vc" CACHE PATH "Path to the build dir of Vc")
file(READ "${Vc_BINARY_DIR}/CMakeCache.txt" _Vc_CMakeCache)
if(_Vc_CMakeCache MATCHES "Vc_SOURCE_DIR:STATIC=([^\n]*)")
   set(Vc_SOURCE_DIR "${CMAKE_MATCH_1}")
   mark_as_advanced(Vc_SOURCE_DIR)

   set(Vc_INCLUDE_DIR "${Vc_SOURCE_DIR}/include;${Vc_SOURCE_DIR}")

   if(NOT Vc_BINARY_DIR STREQUAL _Vc_PREVIOUS_BINARY_DIR)
      set(_Vc_PREVIOUS_BINARY_DIR "${Vc_BINARY_DIR}" CACHE STRING "internal")
      unset(_libVc CACHE)
      unset(_libCpuId CACHE)
      find_library(_libVc Vc HINTS "${Vc_BINARY_DIR}" NO_DEFAULT_PATH)
      find_library(_libCpuId CpuId HINTS "${Vc_BINARY_DIR}" NO_DEFAULT_PATH)
      mark_as_advanced(_Vc_PREVIOUS_BINARY_DIR _libVc _libCpuId)
   endif()
   get_filename_component(Vc_LIB_DIR "${_libVc}" PATH)
   set(Vc_LIBRARIES "${_libVc}")
   if(_libCpuId)
      list(APPEND Vc_LIBRARIES "${_libCpuId}")
   endif()

   set(Vc_CMAKE_MODULES_DIR "${Vc_SOURCE_DIR}/cmake")
   include("${Vc_CMAKE_MODULES_DIR}/VcMacros.cmake")
   set(Vc_DEFINITIONS)
   vc_set_preferred_compiler_flags()

   message(STATUS "Vc at: ${Vc_BINARY_DIR} ${Vc_SOURCE_DIR} ${Vc_INCLUDE_DIR}")
   set(Vc_FOUND true)
endif()
