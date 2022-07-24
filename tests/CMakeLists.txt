enable_testing()
find_package(Catch2 CONFIG REQUIRED)
add_executable(tests 
    main.cpp
    microservice-essentials/context_test.cpp
    )
target_link_libraries(tests PRIVATE Catch2::Catch2 microservice-essentials)

include(CTest)
include(Catch)
catch_discover_tests(tests)
