set(_link_libraries dfa ${GTEST_LIBRARIES})

add_executable(unit_test
        dfa_test.cc
        )
target_link_libraries(unit_test ${_link_libraries})

file(COPY m1.dfa DESTINATION .)

gtest_discover_tests(unit_test)