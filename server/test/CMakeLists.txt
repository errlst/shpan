include_directories(../src)


add_executable(test_sql test_sql.cpp)
target_link_libraries(test_sql sqlite3 gtest)


add_executable(test_sqlpoll test_sqlpoll.cpp ../src/sqlpoll.cpp)
target_link_libraries(test_sqlpoll sqlite3 gtest)


add_executable(test_entry test_entry.cpp ../src/entry.cpp ../src/sqlpoll.cpp)
target_link_libraries(test_entry sqlite3 gtest spdlog fmt)

add_executable(test_fileref test_fileref.cpp ../src/fileref.cpp ../src/sqlpoll.cpp)
target_link_libraries(test_fileref sqlite3 gtest public_lib)