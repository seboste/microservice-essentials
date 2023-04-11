enable_testing()
find_package(Catch2 3 REQUIRED)
find_package(nlohmann_json CONFIG)
add_executable(tests)

if(MSVC)
  target_compile_options(tests PRIVATE /W4 /WX)
else()
  target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(microservice-essentials)
add_subdirectory(utilities)

target_include_directories(tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain nlohmann_json::nlohmann_json microservice-essentials)
include(CTest)
include(Catch)
catch_discover_tests(tests)
