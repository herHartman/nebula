# ---------------------------------------------------------------------------
# Внешние зависимости
# ---------------------------------------------------------------------------
include(FetchContent)

# --- nlohmann_json ---------------------------------------------------------
#
# json торчит в публичных заголовках nebula::http, поэтому он PUBLIC-зависимость:
# потребителю нужны и заголовки, и таргет. Отсюда два требования:
#   1) nebula-config.cmake обязан звать find_dependency(nlohmann_json);
#   2) если json приехал через FetchContent, его надо УСТАНОВИТЬ вместе с
#      nebula - иначе потребитель получит конфиг, который не резолвится.

set(NEBULA_JSON_VERSION "3.12.0" CACHE STRING "Минимальная версия nlohmann_json")

# Системный/vcpkg/conan вариант предпочтительнее: не лезем в сеть и не
# плодим вторую копию библиотеки в чужом дереве.
find_package(nlohmann_json ${NEBULA_JSON_VERSION} QUIET)

if(nlohmann_json_FOUND)
    message(STATUS "nlohmann_json: найден в системе (${nlohmann_json_VERSION})")
    set(NEBULA_JSON_VENDORED OFF CACHE INTERNAL "")
else()
    message(STATUS "nlohmann_json: не найден, качаем ${NEBULA_JSON_VERSION}")
    set(NEBULA_JSON_VENDORED ON CACHE INTERNAL "")

    # URL релизного архива, а не GIT_REPOSITORY: архив 112 КБ против 29 МБ
    # у shallow-клона (в репозитории лежат тестовые данные).
    #
    # EXCLUDE_FROM_ALL здесь НЕ ставим: он выключает и сборку install-правил
    # подпроекта, а нам его install нужен.
    FetchContent_Declare(nlohmann_json
        URL      https://github.com/nlohmann/json/releases/download/v${NEBULA_JSON_VERSION}/json.tar.xz
        URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    set(JSON_BuildTests      OFF CACHE INTERNAL "")
    set(JSON_MultipleHeaders ON  CACHE INTERNAL "")
    # Ставим json ровно тогда, когда ставим саму nebula.
    set(JSON_Install ${NEBULA_INSTALL} CACHE INTERNAL "")

    FetchContent_MakeAvailable(nlohmann_json)
endif()