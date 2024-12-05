add_executable(wait-for-it wait-for-it.cxx)

target_include_directories(wait-for-it PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(wait-for-it PUBLIC core)
