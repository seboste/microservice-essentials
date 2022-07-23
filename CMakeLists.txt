cmake_minimum_required (VERSION 3.16.3)
project (microservice-essentials VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory (src)
add_subdirectory (examples)
#add_subdirectory (tests)