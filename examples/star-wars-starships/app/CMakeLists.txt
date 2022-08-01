add_executable (starships 
    main.cpp
    )

target_link_libraries(starships 
    PUBLIC
        core        
        microservice-essentials
    )
    