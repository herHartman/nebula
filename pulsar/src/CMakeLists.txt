set(PULSAR_SOURCES
    io_context.cpp
    uring_backend.cpp
)

set(PULSAR_HEADERS
    enum_flags.hpp
    io_completion.h
    io_context.hpp
    io_operation.h
    task.hpp
    uring_backend.hpp
    io/acceptor.hpp
    io/basic_socket.hpp
    io/basic_stream_socket.hpp
    io/datagram_socket.hpp
    net/endpoint.hpp
    net/protocol.hpp
    net/tcp.hpp
    net/udp.hpp
)
list(TRANSFORM PULSAR_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/../include/pulsar/")

add_library(pulsar STATIC ${PULSAR_SOURCES} ${PULSAR_HEADERS})
add_library(nebula::pulsar ALIAS pulsar)

set_target_properties(pulsar PROPERTIES
    OUTPUT_NAME pulsar
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME pulsar
)

target_include_directories(pulsar
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(pulsar
    PUBLIC
        Threads::Threads
        liburing::liburing
    PRIVATE
        nebula::warnings
        nebula::options
)

target_compile_features(pulsar PUBLIC cxx_std_23)

# pthread_setaffinity_np / CPU_SET - GNU-расширения
target_compile_definitions(pulsar PRIVATE _GNU_SOURCE)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
if(NEBULA_INSTALL)
    install(TARGETS pulsar
        EXPORT nebula-targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../include/pulsar"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h"
    )
endif()
