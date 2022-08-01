add_library(http-starwars-client)

target_sources(http-starwars-client
    PUBLIC
        http-starwars-client.h
    PRIVATE
        http-starwars-client.cpp
    )

target_link_libraries(http-starwars-client PUBLIC ports)
