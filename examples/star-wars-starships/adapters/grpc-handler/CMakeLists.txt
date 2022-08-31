include(ProtobufGeneration.cmake)

add_library(grpc-handler)

find_package(gRPC CONFIG)

target_protos(grpc-handler api.proto)

target_sources(grpc-handler
    PUBLIC
        grpc-handler.h
    PRIVATE
        grpc-handler.cpp
    )
    
target_link_libraries(grpc-handler 
    PUBLIC 
        ports
    PRIVATE 
        gRPC::gRPC        
    )
