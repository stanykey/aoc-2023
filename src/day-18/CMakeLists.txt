add_executable(lavaduct-lagoon lavaduct-lagoon.cxx)

target_include_directories(lavaduct-lagoon PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lavaduct-lagoon PUBLIC core)
