add_library(http-handler)

find_package(httplib CONFIG)
find_package(nlohmann_json CONFIG)
find_package(OpenSSL CONFIG)

target_sources(http-handler
    PUBLIC
        http-handler.h
    PRIVATE
        http-handler.cpp
    )

target_link_libraries(http-handler 
    PUBLIC 
        ports 
        microservice-essentials
    PRIVATE 
        httplib::httplib 
        nlohmann_json::nlohmann_json
        OpenSSL::OpenSSL
    )
