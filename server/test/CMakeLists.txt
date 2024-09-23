include_directories(../src)

set(DB_PATH ${CMAKE_BINARY_DIR}/server/test/shpan.db)
set(SQL_PATH ${CMAKE_BINARY_DIR}/server/test/init.sql)

execute_process(
    COMMAND rm ${DB_PATH}
    COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/../sql/init.sql ${SQL_PATH}
    COMMAND touch ${DB_PATH}
    COMMAND sh -c "sqlite3 ${DB_PATH} < ${SQL_PATH}"
)

add_compile_options(-DIN_TEST)

add_executable(test_sql test_sql.cpp)
target_link_libraries(test_sql sqlite3 gtest)

add_executable(test_sqlpoll test_sqlpoll.cpp ../src/sqlpoll.cpp)
target_link_libraries(test_sqlpoll sqlite3 gtest)

add_executable(test_fileref test_fileref.cpp)
target_link_libraries(test_fileref gtest server_lib)

add_executable(test_entry test_entry.cpp)
target_link_libraries(test_entry gtest server_lib)

add_executable(test_user test_user.cpp)
target_link_libraries(test_user gtest server_lib)
