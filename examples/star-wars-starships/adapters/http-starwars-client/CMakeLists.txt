add_library(http-starwars-client)

find_package(httplib CONFIG)
find_package(nlohmann_json CONFIG)
find_package(OpenSSL CONFIG)

target_sources(http-starwars-client
    PUBLIC
        http-starwars-client.h
    PRIVATE
        http-starwars-client.cpp
    )

target_link_libraries(http-starwars-client 
    PUBLIC 
        ports
        microservice-essentials
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
        OpenSSL::OpenSSL
    )
