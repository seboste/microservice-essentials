include(CMakeParseArguments)

function(protobuf_gen)
    cmake_parse_arguments(
        PARSED_ARGS # prefix of output variables
        "" # list of names of the boolean arguments (only defined ones will be true)
        "INPUT" # list of names of mono-valued arguments        
        "OUTPUT" # list of names of multi-valued arguments (output variables are lists)
        ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    set(PROTOC_BINARY "${CONAN_BIN_DIRS_PROTOBUF}/protoc")
    if(NOT EXISTS "${PROTOC_BINARY}")
        set(PROTOC_BINARY "${CONAN_BIN_DIRS_PROTOBUF}/protoc.exe")
        if(NOT EXISTS "${PROTOC_BINARY}")
            message(FATAL_ERROR "Cannot find protoc. Did you forget to call find_package(protobuf CONFIG)?")
        endif()
    endif()

    set(GRPC_CPP_PLUGIN_BINARY "${CONAN_BIN_DIRS_GRPC}/grpc_cpp_plugin")
    if(NOT EXISTS "${GRPC_CPP_PLUGIN_BINARY}")
        set(GRPC_CPP_PLUGIN_BINARY "${CONAN_BIN_DIRS_GRPC}/grpc_cpp_plugin.exe")
        if(NOT EXISTS "${GRPC_CPP_PLUGIN_BINARY}")
            message(FATAL_ERROR "Cannot find grpc_cpp_plugin. Did you forget to call find_package(gRPC CONFIG)?")
        endif()
    endif()
    
    set(INPUT "${PARSED_ARGS_INPUT}")
    if(NOT EXISTS "${INPUT}")
        set(INPUT "${CMAKE_CURRENT_LIST_DIR}/${PARSED_ARGS_INPUT}")
        if(NOT EXISTS "${INPUT}")
            message(FATAL_ERROR "${PARSED_ARGS_INPUT} does not exist")
        endif()
    endif()

    get_filename_component(INPUT_NAME ${INPUT} NAME_WE)    

    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    set(OUTPUT "${OUTPUT_DIR}/${INPUT_NAME}.pb.cc" "${OUTPUT_DIR}/${INPUT_NAME}.pb.h" "${OUTPUT_DIR}/${INPUT_NAME}.grpc.pb.cc" "${OUTPUT_DIR}/${INPUT_NAME}.grpc.pb.h")

    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${PROTOC_BINARY} --proto_path=${CMAKE_CURRENT_LIST_DIR} --cpp_out=${OUTPUT_DIR}  --grpc_out=${OUTPUT_DIR} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_BINARY} ${INPUT} 
        DEPENDS ${INPUT}
        VERBATIM
    )
    set(${PARSED_ARGS_OUTPUT} ${OUTPUT} PARENT_SCOPE)

endfunction(protobuf_gen)

#parameters: target protoFile1.proto protoFile2.proto ...
function(target_protos)

    list(GET ARGN 0 TARGET)
    list(REMOVE_AT ARGN 0)

    foreach(protoFile IN LISTS ARGN)
        protobuf_gen(INPUT ${protoFile} OUTPUT generatedSrc)    
        target_sources(${TARGET} PRIVATE ${generatedSrc})    
    endforeach()

endfunction(target_protos)
