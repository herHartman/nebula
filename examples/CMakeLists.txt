set(NEBULA_EXAMPLES
    tcp_echo_server
    tcp_echo_client
    udp_echo_server
)

foreach(example IN LISTS NEBULA_EXAMPLES)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example}
        PRIVATE
            nebula::pulsar
            nebula::warnings
            nebula::options
    )
    set_target_properties(${example} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/examples"
    )
endforeach()
