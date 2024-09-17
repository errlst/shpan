add_subdirectory(test)


add_library(public_lib STATIC fileblob.cpp log.cpp)
target_link_libraries(public_lib cryptopp spdlog fmt)