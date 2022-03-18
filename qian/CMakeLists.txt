include_directories(.)
aux_source_directory(. SRC_LIST)
add_library(qian SHARED ${SRC_LIST})