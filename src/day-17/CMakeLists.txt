add_executable(clumsy-crucible clumsy-crucible.cxx)

target_include_directories(clumsy-crucible PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(clumsy-crucible PUBLIC core)
