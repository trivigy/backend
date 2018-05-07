#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define BOOST_TEST_MODULE test_frontend

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(example_test) { // NOLINT
    BOOST_CHECK_EQUAL(1, 1);
}

#pragma clang diagnostic pop