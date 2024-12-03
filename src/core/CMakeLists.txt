add_library(core STATIC)
target_sources(core PUBLIC io.hxx io.cxx numbers.hxx strings.hxx strings.cxx)

target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/src)
