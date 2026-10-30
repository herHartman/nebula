add_library(pulsar_test_support OBJECT test_support.cpp)
target_link_libraries(pulsar_test_support
    PUBLIC nebula::pulsar GTest::gtest
    PRIVATE nebula::warnings nebula::options
)
target_include_directories(pulsar_test_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

nebula_add_test_suite(
    MODULE pulsar
    TESTS
        task_test
        io_context_test
        net_test
    LIBS
        nebula::pulsar
        pulsar_test_support
)
