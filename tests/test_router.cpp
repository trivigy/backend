#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define BOOST_TEST_MODULE test_router

#include <boost/beast/core.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

#include "server/router.h"

BOOST_AUTO_TEST_CASE(multi_dispatch_test) { // NOLINT
    using boost::beast::http::request;
    using boost::beast::http::response;
    using boost::beast::http::status;
    using boost::beast::http::verb;
    using boost::beast::http::string_body;
    using request_type = request<string_body>;
    using response_type = response<string_body>;

    server::Router router;

    router.add([](request_type &req) -> response_type {
        response_type resp;
        resp.result(status::ok);
        return resp;
    }, "/user");

    router.add([](request_type &req, const int id) -> response_type {
        response_type resp;
        resp.result(status::ok);
        return resp;
    }, "/user/{}", server::params::integer());

    request_type req;
    req.version(11);
    req.method(verb::get);

    req.target("/user");
    BOOST_TEST(router.dispatch(req).result() == status::ok);

    req.target("/user/12345");
    BOOST_TEST(router.dispatch(req).result() == status::ok);
}

#pragma clang diagnostic pop