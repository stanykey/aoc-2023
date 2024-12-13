add_executable(the-floor-will-be-lava the-floor-will-be-lava.cxx)

target_include_directories(the-floor-will-be-lava PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(the-floor-will-be-lava PUBLIC core)
