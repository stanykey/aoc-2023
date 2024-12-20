add_executable(pulse-propagation pulse-propagation.cxx)

target_sources(pulse-propagation
    PUBLIC
        connection-mesh.hxx
        connection-mesh.cxx
)

target_include_directories(pulse-propagation PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pulse-propagation PUBLIC core)
