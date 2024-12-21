add_executable(step-counter step-counter.cxx)

target_include_directories(step-counter PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(step-counter PUBLIC core)
