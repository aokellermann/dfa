set(_link_libraries dfa ${GTEST_LIBRARIES})

add_executable(unit_test
        dfa_test.cc
        )
target_link_libraries(unit_test ${_link_libraries})

gtest_discover_tests(unit_test)