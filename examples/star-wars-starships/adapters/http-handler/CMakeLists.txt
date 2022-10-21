find_package(httplib CONFIG)
find_package(nlohmann_json CONFIG)
find_package(OpenSSL CONFIG)

target_sources(starships
    PRIVATE
        http-handler.h
        http-handler.cpp
    )

target_link_libraries(starships 
    PRIVATE 
        httplib::httplib 
        nlohmann_json::nlohmann_json
        OpenSSL::OpenSSL
    )
