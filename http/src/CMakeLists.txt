# ---------------------------------------------------------------------------
# libhttp - HTTP-слой поверх pulsar
# ---------------------------------------------------------------------------

set(HTTP_SOURCES
    http_headers.cpp
    detail/utils.cpp
)

set(HTTP_HEADERS
    http_headers.h
    http_methods.h
)
list(TRANSFORM HTTP_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/../include/nebula/http/")

add_library(http STATIC ${HTTP_SOURCES} ${HTTP_HEADERS}
    body_reader.cpp)
add_library(nebula::http ALIAS http)

set_target_properties(http PROPERTIES
    OUTPUT_NAME http
    EXPORT_NAME http
)

# Заголовки лежат в http/include/nebula/http/, корень include-пути - это
# http/include, поэтому подключаются как <nebula/http/http_headers.h>.
target_include_directories(http
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(http
    PUBLIC
        # PUBLIC: nlohmann/json.hpp включён в публичный заголовок, поэтому
        # таргет обязан дойти до потребителя. $<BUILD_INTERFACE:> здесь
        # применять НЕЛЬЗЯ - потребитель не соберётся.
        nlohmann_json::nlohmann_json
        nebula::core
        nebula::pulsar
    PRIVATE
        nebula::http_parser
        nebula::warnings
        nebula::options
)

target_compile_features(http PUBLIC cxx_std_23)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
if(NEBULA_INSTALL)
    install(TARGETS http
        EXPORT nebula-targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    # DESTINATION - это РОДИТЕЛЬ копируемого каталога. Чтобы получилось
    # include/nebula/http, нужен include/nebula, а не include.
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../include/nebula/http"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nebula"
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h"
    )
endif()
