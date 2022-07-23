find_package(Catch2 3 REQUIRED)
add_executable(tests microservice-essentials/context_test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain microservice-essentials)
