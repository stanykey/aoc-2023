add_executable(scratchcards scratchcards.cxx)

target_include_directories(scratchcards PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(scratchcards PUBLIC core)
