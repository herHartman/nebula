if(NOT NEBULA_BUILD_TESTS)
    return()
endif()

option(NEBULA_USE_SYSTEM_GTEST "Использовать системный GTest вместо FetchContent" OFF)

if(NEBULA_USE_SYSTEM_GTEST)
    find_package(GTest REQUIRED)
else()
    include(FetchContent)
    set(NEBULA_GTEST_TAG "v1.15.2" CACHE STRING "googletest git tag")

    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG ${NEBULA_GTEST_TAG}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        EXCLUDE_FROM_ALL
    )
    # Не ставить gtest вместе с nebula и не собирать его gmock/тесты.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

add_library(pulsar_test_support OBJECT test_support.cpp)
target_link_libraries(pulsar_test_support
    PUBLIC nebula::pulsar GTest::gtest
    PRIVATE nebula::warnings nebula::options
)
target_include_directories(pulsar_test_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


set(PULSAR_TESTS
    task_test
    io_context_test
    net_test
)

foreach(test IN LISTS PULSAR_TESTS)
    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test}
        PRIVATE
            nebula::pulsar
            nebula::warnings
            nebula::options
            GTest::gtest_main
            pulsar_test_support
    )
    set_target_properties(${test} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/tests"
    )

    gtest_discover_tests(${test}
        DISCOVERY_MODE PRE_TEST
        PROPERTIES
            TIMEOUT 30
            LABELS pulsar
    )
endforeach()