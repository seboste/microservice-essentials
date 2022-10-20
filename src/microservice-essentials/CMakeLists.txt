add_subdirectory(cross-cutting-concerns)
add_subdirectory(observability)
add_subdirectory(request)
add_subdirectory(security)
add_subdirectory(utilities)

target_sources(microservice-essentials
    PUBLIC
        context.h        
        handler.h
        status.h
    PRIVATE
        context.cpp
        handler.cpp
        status.cpp
)