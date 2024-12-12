add_executable(lens-library lens-library.cxx)

target_include_directories(lens-library PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lens-library PUBLIC core)
