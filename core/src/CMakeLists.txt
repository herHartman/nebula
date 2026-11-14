set(CORE_SOURCES
)

set(CORE_HEADERS
    reflection.hpp
)

list(TRANSFORM CORE_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/../include/nebula/core/")

add_library(core STATIC ${HTTP_SOURCES} ${HTTP_HEADERS})
add_library(nebula::core ALIAS http)

set_target_properties(core PROPERTIES
    OUTPUT_NAME core
    EXPORT_NAME core
)

# Заголовки лежат в core/include/nebula/http/, корень include-пути - это
# core/include, поэтому подключаются как <nebula/http/http_headers.h>.
target_include_directories(core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(core
    PRIVATE
        nebula::warnings
        nebula::options
)

target_compile_features(core PUBLIC cxx_std_23)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
if(NEBULA_INSTALL)
    install(TARGETS core
        EXPORT nebula-targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    # DESTINATION - это РОДИТЕЛЬ копируемого каталога. Чтобы получилось
    # include/nebula/core, нужен include/nebula, а не include.
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../include/nebula/core"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nebula"
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h"
    )
endif()
