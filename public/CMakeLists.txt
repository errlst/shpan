add_subdirectory(test)
add_subdirectory(proto)

include_directories(proto)

add_library(public_lib STATIC fileblob.cpp log.cpp)
target_link_libraries(public_lib proto_lib cryptopp spdlog fmt)
