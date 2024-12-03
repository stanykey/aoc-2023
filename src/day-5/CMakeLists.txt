add_executable(seed-fertilizer seed-fertilizer.cxx)

target_include_directories(seed-fertilizer PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(seed-fertilizer PUBLIC core)
