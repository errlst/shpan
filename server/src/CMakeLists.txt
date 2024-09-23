add_library(server_lib STATIC
    sqlpoll.cpp
    entry.cpp
    fileref.cpp
    user.cpp
)

target_link_libraries(server_lib sqlite3 public_lib)


add_executable(server server.cpp)

