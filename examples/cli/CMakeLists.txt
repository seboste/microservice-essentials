add_executable (cli 
    main.cpp
    )

target_link_libraries(cli 
    PUBLIC microservice-essentials
    )