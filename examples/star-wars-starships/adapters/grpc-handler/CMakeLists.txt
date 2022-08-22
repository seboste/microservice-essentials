add_library(grpc-handler)

find_package(gRPC CONFIG)
find_package(nlohmann_json CONFIG)
find_package(OpenSSL CONFIG)

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
