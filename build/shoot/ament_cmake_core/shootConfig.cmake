# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_shoot_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED shoot_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(shoot_FOUND FALSE)
  elseif(NOT shoot_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(shoot_FOUND FALSE)
  endif()
  return()
endif()
set(_shoot_CONFIG_INCLUDED TRUE)

# output package information
if(NOT shoot_FIND_QUIETLY)
  message(STATUS "Found shoot: 0.0.0 (${shoot_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'shoot' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${shoot_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(shoot_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "ament_cmake_export_dependencies-extras.cmake")
foreach(_extra ${_extras})
  include("${shoot_DIR}/${_extra}")
endforeach()
