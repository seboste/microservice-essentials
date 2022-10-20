set(STARSHIP_EXAMPLE_DIR ${CMAKE_CURRENT_LIST_DIR})

add_executable(starships)

target_include_directories(starships
    PRIVATE
        ${STARSHIP_EXAMPLE_DIR}
    )

add_subdirectory(adapters)
add_subdirectory(app)
add_subdirectory(core)
add_subdirectory(ports)

target_link_libraries(starships 
    PUBLIC
        microservice-essentials
    )
    

