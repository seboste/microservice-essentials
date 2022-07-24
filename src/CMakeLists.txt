add_library(microservice-essentials)
add_subdirectory (microservice-essentials)
target_include_directories(microservice-essentials PUBLIC ${CMAKE_CURRENT_LIST_DIR})

install(TARGETS microservice-essentials DESTINATION lib)

#TODO: upgrade to cmake 3.23 and use FILE_SET 
#see https://stackoverflow.com/questions/10487256/cmake-how-to-properly-copy-static-librarys-header-file-into-usr-include
install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/src/microservice-essentials  
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    )
