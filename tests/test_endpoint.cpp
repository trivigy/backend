#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define BOOST_TEST_MODULE test_view

#include <boost/test/unit_test.hpp>

#include "common/endpoint.h"

using common::Endpoint;

BOOST_AUTO_TEST_CASE(construct_invalid_endpoint) { // NOLINT
    BOOST_CHECK_THROW(Endpoint("I am not a valid URI."), invalid_argument);
}

BOOST_AUTO_TEST_CASE(construct_uri_from_char_array) { // NOLINT
    BOOST_CHECK_NO_THROW(Endpoint("http://www.example.com"));
}

BOOST_AUTO_TEST_CASE(construct_default) { // NOLINT
    Endpoint u;
    BOOST_TEST(u.scheme() == "");
    BOOST_TEST(u.host() == "");
    BOOST_TEST(u.port() == 0);
}

BOOST_AUTO_TEST_CASE(construct_explicit) { // NOLINT
    Endpoint u("scheme://www.example.com:8080");
    BOOST_TEST(u.scheme() == "scheme");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_host_port) { // NOLINT
    Endpoint u("www.example.com", 8080);
    BOOST_TEST(u.scheme() == "");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_scheme_host_port) { // NOLINT
    Endpoint u("scheme", "www.example.com", 8080);
    BOOST_TEST(u.scheme() == "scheme");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_special_scheme) { // NOLINT
    Endpoint u("https://www.example.com");
    BOOST_TEST(u.scheme() == "https");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 443);
}

BOOST_AUTO_TEST_CASE(construct_special_scheme_override) { // NOLINT
    Endpoint u("https://www.example.com:8443");
    BOOST_TEST(u.scheme() == "https");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8443);
}

BOOST_AUTO_TEST_CASE(verify_type_ipv4) { // NOLINT
    Endpoint u("scheme://198.51.100.0");
    BOOST_TEST(u.is_ipv4());
}

BOOST_AUTO_TEST_CASE(verify_type_ipv6) { // NOLINT
    Endpoint u("scheme://[2001:db8:85a3:0:0:8a2e:370:7334]");
    BOOST_TEST(u.is_ipv6());
}

BOOST_AUTO_TEST_CASE(verify_type_fqdn) { // NOLINT
    Endpoint u("scheme://www.example.com");
    BOOST_TEST(u.is_fqdn());
}

BOOST_AUTO_TEST_CASE(print_compose_well_formed) { // NOLINT
    Endpoint u("scheme://www.example.com:8080");
    BOOST_TEST(u.compose() == "scheme://www.example.com:8080");
}

BOOST_AUTO_TEST_CASE(print_compose_without_port) { // NOLINT
    Endpoint u("scheme://www.example.com");
    BOOST_TEST(u.compose() == "scheme://www.example.com");
}

BOOST_AUTO_TEST_CASE(print_compose_without_scheme) { // NOLINT
    Endpoint u("www.example.com:8080");
    BOOST_TEST(u.compose() == "www.example.com:8080");
}

BOOST_AUTO_TEST_CASE(print_compose_special_scheme) { // NOLINT
    Endpoint u("https://www.example.com");
    BOOST_TEST(u.compose() == "https://www.example.com");
}

BOOST_AUTO_TEST_CASE(print_compose_special_scheme_override) { // NOLINT
    Endpoint u("https://www.example.com:8443");
    BOOST_TEST(u.compose() == "https://www.example.com:8443");
}

BOOST_AUTO_TEST_CASE(print_compose_ipv6) { // NOLINT
    Endpoint u("scheme://[2001:DB8::1]:8443");
    BOOST_TEST(u.compose() == "scheme://[2001:DB8::1]:8443");
}

BOOST_AUTO_TEST_CASE(print_netloc_with_port) { // NOLINT
    Endpoint u("scheme://www.example.com:8080");
    BOOST_TEST(u.netloc() == "www.example.com:8080");
}

BOOST_AUTO_TEST_CASE(print_netloc_without_port) { // NOLINT
    Endpoint u("scheme://www.example.com");
    BOOST_TEST(u.netloc() == "www.example.com");
}

#pragma clang diagnostic pop