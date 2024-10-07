file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(FILE_NAME ${PROTO_FILE} NAME)
    execute_process(
        COMMAND protoc -I=${CMAKE_CURRENT_SOURCE_DIR} ${FILE_NAME} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

file(GLOB cc_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_library(proto_lib STATIC ${cc_files})
target_link_libraries(proto_lib protobuf)
