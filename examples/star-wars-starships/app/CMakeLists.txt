target_sources(starships 
    PRIVATE
        main.cpp
    )

target_link_libraries(starships 
    PUBLIC    
        microservice-essentials
    )
    