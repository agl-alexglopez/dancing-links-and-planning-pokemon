cmake_minimum_required(VERSION 3.20)
project(dancing-links-and-planning-pokemon CXX)

if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Project must be built outside its source directory, e.g. `cmake -B build`.")
endif()

include(etc/build_type.cmake)
include(etc/scanners.cmake)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/demos")
include_directories("${PROJECT_SOURCE_DIR}/gui")
include_directories("${PROJECT_SOURCE_DIR}/tests")

add_subdirectory("${PROJECT_SOURCE_DIR}/src")
add_subdirectory("${PROJECT_SOURCE_DIR}/demos")
add_subdirectory("${PROJECT_SOURCE_DIR}/gui")
add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
