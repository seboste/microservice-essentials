add_library(dummy-starwars-client)

target_sources(dummy-starwars-client
    PUBLIC
    dummy-starwars-client.h
    PRIVATE
    dummy-starwars-client.cpp
    )

target_link_libraries(dummy-starwars-client PUBLIC ports)
