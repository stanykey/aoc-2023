add_executable(cube-conundrum cube-conundrum.cxx)

target_include_directories(cube-conundrum PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(cube-conundrum PUBLIC core)
