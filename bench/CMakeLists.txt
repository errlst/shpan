include_directories(../3rd/sqlite3pp/headeronly_src)
include_directories(../server/src)
add_executable(1.out 1.cpp)
target_link_libraries(1.out benchmark server_lib)
