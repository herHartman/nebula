include_guard(GLOBAL)

if(NEBULA_USE_SYSTEM_GTEST)
    find_package(GTest REQUIRED)
else()
    include(FetchContent)
    set(NEBULA_GTEST_TAG "v1.15.2" CACHE STRING "googletest git tag")

    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        ${NEBULA_GTEST_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

# nebula_add_test_suite(
#     MODULE <имя>            цель <имя>_tests, префикс в ctest и лейбл
#     TESTS  <file> ...       исходники тестов без .cpp
#     LIBS   <target> ...     что линковать (модуль, support-библиотеки)
# )
function(nebula_add_test_suite)
    cmake_parse_arguments(ARG "" "MODULE" "TESTS;LIBS" ${ARGN})

    if(NOT ARG_MODULE)
        message(FATAL_ERROR "nebula_add_test_suite: MODULE is required")
    endif()

    set(target ${ARG_MODULE}_tests)
    list(TRANSFORM ARG_TESTS APPEND ".cpp" OUTPUT_VARIABLE sources)

    add_executable(${target} ${sources})
    target_link_libraries(${target}
        PRIVATE
            ${ARG_LIBS}
            nebula::warnings
            nebula::options
            GTest::gtest_main
    )
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/tests"
    )

    gtest_discover_tests(${target}
        DISCOVERY_MODE PRE_TEST
        TEST_PREFIX "${ARG_MODULE}."
        PROPERTIES
            TIMEOUT 30
            LABELS ${ARG_MODULE}
    )
endfunction()
