add_executable (starships 
    main.cpp
    )

target_link_libraries(starships 
    PUBLIC
        core
        in-memory-status-db
        http-starwars-client 
        microservice-essentials
    )
    