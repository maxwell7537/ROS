# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_project1_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED project1_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(project1_FOUND FALSE)
  elseif(NOT project1_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(project1_FOUND FALSE)
  endif()
  return()
endif()
set(_project1_CONFIG_INCLUDED TRUE)

# output package information
if(NOT project1_FIND_QUIETLY)
  message(STATUS "Found project1: 0.0.0 (${project1_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'project1' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${project1_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(project1_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "ament_cmake_export_dependencies-extras.cmake;ament_cmake_export_include_directories-extras.cmake")
foreach(_extra ${_extras})
  include("${project1_DIR}/${_extra}")
endforeach()
