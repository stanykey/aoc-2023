add_executable(mirage-maintenance mirage-maintenance.cxx)

target_include_directories(mirage-maintenance PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(mirage-maintenance PUBLIC core)
