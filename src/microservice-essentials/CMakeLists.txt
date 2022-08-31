add_subdirectory(cross-cutting-concerns)

target_sources(microservice-essentials
    PUBLIC
        context.h
        handler.h
    PRIVATE
        context.cpp
        handler.cpp
)