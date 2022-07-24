cmake_minimum_required (VERSION 3.16.3)
project (microservice-essentials VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

add_subdirectory (src)

if(BUILD_TESTS)
    add_subdirectory (tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory (examples)
endif()
