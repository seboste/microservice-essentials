add_subdirectory(cross-cutting-concerns)
add_subdirectory(utilities)

target_sources(microservice-essentials
    PUBLIC
        context.h
        handler.h
    PRIVATE
        context.cpp
        handler.cpp
)