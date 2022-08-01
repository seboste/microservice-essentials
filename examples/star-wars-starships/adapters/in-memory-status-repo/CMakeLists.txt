add_library(in-memory-status-repo)

target_sources(in-memory-status-repo
    PUBLIC
        in-memory-status-repo.h
    PRIVATE
        in-memory-status-repo.cpp
    )

target_link_libraries(in-memory-status-repo PUBLIC ports)
