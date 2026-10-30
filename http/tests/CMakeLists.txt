nebula_add_test_suite(
    MODULE http
    TESTS
        percent_decode_test
    #   parser_test
    #   router_test
    #   server_test
    LIBS
        nebula::http
        pulsar_test_support
)
