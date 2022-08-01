add_library(in-memory-status-db)

target_sources(in-memory-status-db
    PUBLIC
        in-memory-status-db.h
    PRIVATE
        in-memory-status-db.cpp
    )

target_link_libraries(in-memory-status-db PUBLIC ports)
