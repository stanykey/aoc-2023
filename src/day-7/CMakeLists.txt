add_executable(camel-cards camel-cards.cxx)

target_include_directories(camel-cards PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(camel-cards PUBLIC core)
