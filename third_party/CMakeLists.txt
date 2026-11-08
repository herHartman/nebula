if(NOT CMAKE_C_COMPILER_LOADED)
    enable_language(C)
endif()

add_library(http_parser STATIC http_parser/http_parser.c)
add_library(nebula::http_parser ALIAS http_parser)

set_target_properties(http_parser PROPERTIES
    EXPORT_NAME http_parser
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(http_parser
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/http_parser>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nebula/http_parser>
)

target_compile_features(http_parser PRIVATE c_std_11)

if(NEBULA_INSTALL)
    install(TARGETS http_parser
        EXPORT nebula-targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nebula/http_parser"
    )
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/http_parser/http_parser.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nebula/http_parser"
    )
endif()
