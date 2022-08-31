cmake_minimum_required (VERSION 3.16.3)

if(CMAKE_BUILD_TYPE)
set(_BTYPE ${CMAKE_BUILD_TYPE})
elseif(NOT BUILD_TYPE STREQUAL "")
set(_BTYPE ${BUILD_TYPE})
endif()

message("_BTYPE: ${_BTYPE}")

if(${_BTYPE} MATCHES "Release|_RELEASE")
    message("build type matched")
endif()

if(APPLE)
    find_library(LIBRARY_COREFOUNDATION CoreFoundation)
    message("LIBRARY_COREFOUNDATION: ${LIBRARY_COREFOUNDATION}")
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

project (microservice-essentials VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

option(BUILD_TESTING "build the tests for microservice-essentials lib" OFF)
option(BUILD_EXAMPLES "build an example microservice using the microservice-essentials" OFF)

add_subdirectory (src)

if(BUILD_TESTING)    
    add_subdirectory (tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory (examples)
endif()
