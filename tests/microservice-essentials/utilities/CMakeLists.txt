target_sources(tests
PUBLIC
    environment_test.cpp
    metadata-converter_test.cpp
    # disabled flaky test on MacOS 
    # signal-handler_test.cpp
    status-converter_test.cpp
    url_test.cpp
    )
