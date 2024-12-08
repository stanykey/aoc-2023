add_executable(cosmic-expansion cosmic-expansion.cxx)

target_include_directories(cosmic-expansion PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(cosmic-expansion PUBLIC core)
