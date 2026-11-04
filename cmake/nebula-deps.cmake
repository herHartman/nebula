include(FetchContent)

set(NEBULA_JSON_VERSION "3.12.0" CACHE STRING "Минимальная версия nlohmann_json")

find_package(nlohmann_json ${NEBULA_JSON_VERSION} QUIET)

if(nlohmann_json_FOUND)
    message(STATUS "nlohmann_json: найден в системе (${nlohmann_json_VERSION})")
    set(NEBULA_JSON_VENDORED OFF CACHE INTERNAL "")
else()
    message(STATUS "nlohmann_json: не найден, качаем ${NEBULA_JSON_VERSION}")
    set(NEBULA_JSON_VENDORED ON CACHE INTERNAL "")

    FetchContent_Declare(nlohmann_json
        URL      https://github.com/nlohmann/json/releases/download/v${NEBULA_JSON_VERSION}/json.tar.xz
        URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    set(JSON_BuildTests      OFF CACHE INTERNAL "")
    set(JSON_MultipleHeaders ON  CACHE INTERNAL "")
    set(JSON_Install ${NEBULA_INSTALL} CACHE INTERNAL "")

    FetchContent_MakeAvailable(nlohmann_json)
endif()
