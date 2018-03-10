#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define BOOST_TEST_MODULE test_router

#include <boost/test/unit_test.hpp>
#include <iostream>

#include "server/router.h"

BOOST_AUTO_TEST_CASE(multi_dispatch_test) { // NOLINT
    server::Router router;

    router.add([] {
    }, "/user");

    router.add([](const int id) {
    }, "/user/{}", server::params::integer());

    BOOST_TEST(router.dispatch("/user"));
    BOOST_TEST(router.dispatch("/user/12345"));
}

#pragma clang diagnostic pop