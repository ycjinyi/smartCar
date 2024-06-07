aux_source_directory(. SRC_LIST)
add_executable(car ${SRC_LIST})
target_link_libraries(car pthread)