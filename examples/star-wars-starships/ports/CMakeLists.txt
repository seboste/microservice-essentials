add_library(ports INTERFACE)

target_sources(ports
    INTERFACE
        api.h
        model.h
        starwars-client.h
        status-db.h
    )

target_include_directories(ports INTERFACE ${STARSHIP_EXAMPLE_DIR})