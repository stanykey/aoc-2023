add_executable(point-of-incidence point-of-incidence.cxx)

target_include_directories(point-of-incidence PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(point-of-incidence PUBLIC core)
