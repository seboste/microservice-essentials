add_library(ports INTERFACE)

target_sources(ports
    INTERFACE
        service.h
        starwars-client.h
    )

target_include_directories(ports INTERFACE ${STARSHIP_EXAMPLE_DIR})