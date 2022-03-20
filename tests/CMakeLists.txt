add_executable(test_timestamp test_timestamp.cpp)
target_link_libraries(test_timestamp qian)

add_executable(test_buffer test_buffer.cpp)
target_link_libraries(test_buffer qian)

add_executable(test_thread test_thread.cpp)
target_link_libraries(test_thread qian)