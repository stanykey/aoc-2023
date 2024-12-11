add_executable(parabolic-reflector-dish parabolic-reflector-dish.cxx)

target_include_directories(parabolic-reflector-dish
                           PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(parabolic-reflector-dish PUBLIC core)
