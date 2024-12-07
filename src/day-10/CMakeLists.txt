add_executable(pipe-maze pipe-maze.cxx)

target_include_directories(pipe-maze PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pipe-maze PUBLIC core)
