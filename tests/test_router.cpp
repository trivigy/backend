#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define BOOST_TEST_MODULE test_dispatcher

#include <boost/test/unit_test.hpp>
#include <iostream>

#include "server/router.h"

class F : public server::Router {
};

BOOST_FIXTURE_TEST_CASE(remove_duplicates_test, F) { // NOLINT
    using namespace std;
    using server::matchers::word;
    using server::matchers::integer;

    add(server::matcher("/user/") + integer(),
        [] (auto id) {
            cerr << "/user/" << to_string(id) << endl;
        });

    add(server::matcher("/resource/") + word(),
        [] (auto name) {
            cerr << "/resource/" << name << endl;
        });

    add(server::matcher("/foo/") + integer() + "/bar/" + word(),
        [] (auto id, auto name) {
            cerr << "/foo/" << to_string(id) << "/bar/" << name << endl;
        });

    dispatch("/user/1234");
    dispatch("/resource/john_doe");
    dispatch("/foo/42/bar/jane_smith/");
    dispatch("/user/admin");
}

#pragma clang diagnostic pop