add_library(core)

target_sources(core
    PUBLIC
        core.h
    PRIVATE
        core.cpp
    )

target_link_libraries(core PUBLIC ports)


