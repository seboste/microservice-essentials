add_library(microservice-essentials)

if(MSVC)
  target_compile_options(microservice-essentials PRIVATE /W4 /WX)
else()
  target_compile_options(microservice-essentials PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()


add_subdirectory (microservice-essentials)
target_include_directories(microservice-essentials PUBLIC ${CMAKE_CURRENT_LIST_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(microservice-essentials PRIVATE Threads::Threads)

install(TARGETS microservice-essentials DESTINATION lib)

#TODO: upgrade to cmake 3.23 and use FILE_SET 
#see https://stackoverflow.com/questions/10487256/cmake-how-to-properly-copy-static-librarys-header-file-into-usr-include
install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/src/microservice-essentials  
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    )
