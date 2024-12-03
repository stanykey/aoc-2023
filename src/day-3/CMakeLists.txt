add_executable(gear-ratios gear-ratios.cxx)

target_include_directories(gear-ratios PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(gear-ratios PUBLIC core)
