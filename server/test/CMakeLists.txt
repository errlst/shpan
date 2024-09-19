include_directories(../src)

add_compile_options(-DTEST)

add_executable(test_sql test_sql.cpp)
target_link_libraries(test_sql sqlite3 gtest)


add_executable(test_sqlpoll test_sqlpoll.cpp ../src/sqlpoll.cpp)
target_link_libraries(test_sqlpoll sqlite3 gtest)


add_executable(test_entry test_entry.cpp ../src/entry.cpp ../src/sqlpoll.cpp)
target_link_libraries(test_entry gtest server_lib)

add_executable(test_fileref test_fileref.cpp ../src/fileref.cpp ../src/sqlpoll.cpp)
target_link_libraries(test_fileref gtest server_lib)
