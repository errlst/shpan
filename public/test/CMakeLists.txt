include_directories(${CMAKE_SOURCE_DIR}/public)



# 生成随机文件
execute_process(
    COMMAND dd if=/dev/random of=${CMAKE_BINARY_DIR}/public/test/fileblob_test_100M bs=1M count=100
    RESULT_VARIABLE result
    ERROR_VARIABLE err
    OUTPUT_QUIET
    ERROR_QUIET
)

execute_process(
    COMMAND dd if=/dev/random of=${CMAKE_BINARY_DIR}/public/test/fileblob_test_1M bs=1M count=1
    RESULT_VARIABLE result
    ERROR_VARIABLE err
    OUTPUT_QUIET
    ERROR_QUIET
)

execute_process(
    COMMAND dd if=/dev/random of=${CMAKE_BINARY_DIR}/public/test/fileblob_test_1K bs=1K count=1
    RESULT_VARIABLE result
    ERROR_VARIABLE err
    OUTPUT_QUIET
    ERROR_QUIET
)

add_executable(test_fileblob test_fileblob.cpp ${CMAKE_SOURCE_DIR}/public/fileblob.cpp)
target_link_libraries(test_fileblob cryptopp)


add_executable(test_log test_log.cpp ../log.cpp)
target_link_libraries(test_log gtest spdlog fmt)