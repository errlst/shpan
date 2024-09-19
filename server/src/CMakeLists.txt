add_library(server_lib
    sqlpoll.cpp
    entry.cpp
    fileref.cpp
)

target_link_libraries(server_lib sqlite3 public_lib)
