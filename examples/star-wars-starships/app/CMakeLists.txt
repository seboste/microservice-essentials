add_executable (starships 
    main.cpp
    )

target_link_libraries(starships 
    PUBLIC
        core        
        dummy-starwars-client
        http-handler
        http-starwars-client 
        in-memory-status-db
        microservice-essentials
    )
    