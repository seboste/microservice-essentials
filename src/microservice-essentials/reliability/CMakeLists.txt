target_sources(microservice-essentials
    PUBLIC
        circuit-breaker-request-hook.h
        retry-request-hook.h
    PRIVATE
        circuit-breaker-request-hook.cpp
        retry-request-hook.cpp
)