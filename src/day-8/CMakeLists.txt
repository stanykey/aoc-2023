add_executable(haunted-wasteland haunted-wasteland.cxx)

target_include_directories(haunted-wasteland PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(haunted-wasteland PUBLIC core)
