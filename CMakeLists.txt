cmake_minimum_required(VERSION 3.28)

project(nebula
    VERSION 0.1.0
    DESCRIPTION "Lightweight async I/O engine based on io_uring with coroutines"
    HOMEPAGE_URL "https://github.com/example/nebula"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CMakeDependentOption)

cmake_dependent_option(NEBULA_BUILD_EXAMPLES "Build examples" ON "PROJECT_IS_TOP_LEVEL" OFF)
cmake_dependent_option(NEBULA_BUILD_TESTS    "Build tests"    ON "PROJECT_IS_TOP_LEVEL" OFF)

option(NEBULA_USE_SYSTEM_GTEST  "Использовать системный GTest вместо FetchContent" OFF)
option(NEBULA_BUILD_HTTP        "Build the HTTP layer"                  ON)
option(NEBULA_ENABLE_SANITIZERS "Enable ASan+UBSan"                     OFF)
option(NEBULA_ENABLE_TSAN       "Enable ThreadSanitizer (xor с ASan)"   OFF)
option(NEBULA_ENABLE_WERROR     "Treat warnings as errors"              OFF)
option(NEBULA_NATIVE_ARCH       "Build with -march=native (не переносимо!)" OFF)
option(NEBULA_INSTALL           "Generate install/export rules"         ${PROJECT_IS_TOP_LEVEL})

if(NEBULA_ENABLE_SANITIZERS AND NEBULA_ENABLE_TSAN)
    message(FATAL_ERROR "NEBULA_ENABLE_SANITIZERS (ASan) и NEBULA_ENABLE_TSAN несовместимы - выберите что-то одно")
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            Debug Release RelWithDebInfo MinSizeRel)
        message(STATUS "CMAKE_BUILD_TYPE не задан - используется RelWithDebInfo")
    endif()

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "nebula использует io_uring и собирается только под Linux (текущая цель: ${CMAKE_SYSTEM_NAME})")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(liburing REQUIRED)

include(nebula-deps)

add_library(nebula_warnings INTERFACE)
add_library(nebula::warnings ALIAS nebula_warnings)
target_compile_options(nebula_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wconversion
    -Wsign-conversion
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond>
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
    $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
    $<$<CXX_COMPILER_ID:GNU>:-Wuseless-cast>
    $<$<BOOL:${NEBULA_ENABLE_WERROR}>:-Werror>
)

add_library(nebula_options INTERFACE)
add_library(nebula::options ALIAS nebula_options)

if(NEBULA_NATIVE_ARCH)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-march=native NEBULA_HAS_MARCH_NATIVE)
    if(NEBULA_HAS_MARCH_NATIVE)
        target_compile_options(nebula_options INTERFACE
            $<$<COMPILE_LANGUAGE:CXX>:-march=native>)
    else()
        message(WARNING "-march=native is not supported, ignore")
    endif()
endif()

if(NEBULA_ENABLE_SANITIZERS)
    target_compile_options(nebula_options INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=undefined
    )
    target_link_options(nebula_options INTERFACE -fsanitize=address,undefined)
endif()

if(NEBULA_ENABLE_TSAN)
    target_compile_options(nebula_options INTERFACE -fsanitize=thread -fno-omit-frame-pointer)
    target_link_options(nebula_options INTERFACE -fsanitize=thread)
endif()

if(NEBULA_BUILD_TESTS)
    enable_testing()
    include(NebulaTesting)
endif()

add_subdirectory(core)
add_subdirectory(pulsar)

if(NEBULA_BUILD_HTTP)
    add_subdirectory(third_party)
    add_subdirectory(http)
endif()

if(NEBULA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(NEBULA_BUILD_TESTS)
    add_subdirectory(pulsar/tests)
    if(NEBULA_BUILD_HTTP)
        add_subdirectory(http/tests)
    endif()
endif()

# ---------------------------------------------------------------------------
# Install / export
# ---------------------------------------------------------------------------
if(NEBULA_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS nebula_warnings nebula_options
        EXPORT nebula-targets
    )

    install(EXPORT nebula-targets
        FILE nebula-targets.cmake
        NAMESPACE nebula::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nebula"
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/nebula-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/nebula-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nebula"
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/nebula-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/nebula-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/nebula-config-version.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findliburing.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nebula"
    )
endif()

message(STATUS "nebula ${PROJECT_VERSION}: build=${CMAKE_BUILD_TYPE} http=${NEBULA_BUILD_HTTP} examples=${NEBULA_BUILD_EXAMPLES} tests=${NEBULA_BUILD_TESTS} asan=${NEBULA_ENABLE_SANITIZERS}")
