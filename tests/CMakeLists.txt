 # tests for eventloop
add_executable(test_eventloop1 test_eventloop_1.cc)
target_link_libraries(test_eventloop1 qian)
add_executable(test_eventloop2 test_eventloop_2.cc)
target_link_libraries(test_eventloop2 qian)

# test reactor
add_executable(test_reactor test_reactor.cc)
target_link_libraries(test_reactor qian)
