include(ProtobufGeneration.cmake)

find_package(gRPC CONFIG)

add_library(generated-grpc-handler)
target_protos(generated-grpc-handler api.proto)
target_link_libraries(generated-grpc-handler)

target_sources(starships    
    PRIVATE
        grpc-handler.h
        grpc-handler.cpp
    )
    
target_link_libraries(starships    
    PRIVATE
        generated-grpc-handler        
        microservice-essentials
    )
