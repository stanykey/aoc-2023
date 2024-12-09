add_executable(hot-springs hot-springs.cxx)

target_include_directories(hot-springs PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(hot-springs PUBLIC core)
