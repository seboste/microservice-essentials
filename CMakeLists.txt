cmake_minimum_required (VERSION 3.16.3)

project (microservice-essentials VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

option(BUILD_TESTING "build the tests for microservice-essentials lib" OFF)
option(BUILD_EXAMPLES "build an example microservice using the microservice-essentials" OFF)

add_subdirectory (src)

if(BUILD_TESTING OR BUILD_EXAMPLES)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
endif()

if(BUILD_TESTING)    
    add_subdirectory (tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory (examples)
endif()
