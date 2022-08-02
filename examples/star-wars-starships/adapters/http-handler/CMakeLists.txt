add_library(http-handler)

find_package(httplib CONFIG)

target_sources(http-handler
    PUBLIC
        http-handler.h
    PRIVATE
        http-handler.cpp
    )

target_link_libraries(http-handler PUBLIC ports PRIVATE httplib::httplib)
