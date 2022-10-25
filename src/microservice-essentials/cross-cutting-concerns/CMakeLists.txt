target_sources(microservice-essentials
    PUBLIC
        exception-handling-request-hook.h
        graceful-shutdown.h
    PRIVATE
        exception-handling-request-hook.cpp
        graceful-shutdown.cpp        
)