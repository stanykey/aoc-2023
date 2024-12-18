add_executable(aplenty aplenty.cxx)

target_include_directories(aplenty PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(aplenty PUBLIC core)
