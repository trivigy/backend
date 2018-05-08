#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define BOOST_TEST_MODULE test_uri

#include <boost/test/unit_test.hpp>

#include "common/uri.h"

using common::Uri;

BOOST_AUTO_TEST_CASE(construct_invalid_uri) { // NOLINT
    BOOST_CHECK_THROW(Uri("I am not a valid URI."), invalid_argument);
}

BOOST_AUTO_TEST_CASE(construct_uri_from_char_array) { // NOLINT
    BOOST_CHECK_NO_THROW(Uri("http://www.example.com"));
}

BOOST_AUTO_TEST_CASE(construct_default) { // NOLINT
    Uri u;
    BOOST_TEST(u.scheme() == "");
    BOOST_TEST(u.host() == "");
    BOOST_TEST(u.port() == 0);
}

BOOST_AUTO_TEST_CASE(construct_explicit) { // NOLINT
    Uri u("scheme://www.example.com:8080");
    BOOST_TEST(u.scheme() == "scheme");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_explicit_username) { // NOLINT
    Uri u("scheme://username@www.example.com:8080");
    BOOST_TEST(u.scheme() == "scheme");
    BOOST_TEST(u.username() == "username");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_explicit_basic_auth) { // NOLINT
    Uri u("scheme://username:password@www.example.com:8080");
    BOOST_TEST(u.scheme() == "scheme");
    BOOST_TEST(u.username() == "username");
    BOOST_TEST(u.password() == "password");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_basic_auth_without_scheme) { // NOLINT
    Uri u("username:password@www.example.com");
    BOOST_TEST(u.username() == "username");
    BOOST_TEST(u.password() == "password");
    BOOST_TEST(u.host() == "www.example.com");
}

BOOST_AUTO_TEST_CASE(construct_host_port) { // NOLINT
    Uri u("www.example.com", 8080);
    BOOST_TEST(u.scheme() == "");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_scheme_host_port) { // NOLINT
    Uri u("scheme", "www.example.com", 8080);
    BOOST_TEST(u.scheme() == "scheme");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8080);
}

BOOST_AUTO_TEST_CASE(construct_special_scheme) { // NOLINT
    Uri u("https://www.example.com");
    BOOST_TEST(u.scheme() == "https");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 443);
}

BOOST_AUTO_TEST_CASE(construct_special_scheme_override) { // NOLINT
    Uri u("https://www.example.com:8443");
    BOOST_TEST(u.scheme() == "https");
    BOOST_TEST(u.host() == "www.example.com");
    BOOST_TEST(u.port() == 8443);
}

BOOST_AUTO_TEST_CASE(construct_localhost) { // NOLINT
    BOOST_CHECK_NO_THROW(Uri("scheme://localhost"));
}

BOOST_AUTO_TEST_CASE(verify_type_ipv4) { // NOLINT
    Uri u("scheme://198.51.100.0");
    BOOST_TEST(u.is_ipv4());
}

BOOST_AUTO_TEST_CASE(verify_type_ipv6) { // NOLINT
    Uri u("scheme://[2001:db8:85a3:0:0:8a2e:370:7334]");
    BOOST_TEST(u.is_ipv6());
}

BOOST_AUTO_TEST_CASE(verify_type_fqdn) { // NOLINT
    Uri u("scheme://www.example.com");
    BOOST_TEST(u.is_fqdn());
}

BOOST_AUTO_TEST_CASE(print_compose_well_formed) { // NOLINT
    Uri u("scheme://www.example.com:8080/sample/path?query=11&query2#fragment");
    BOOST_TEST(u.compose() == "scheme://www.example.com:8080/sample/path?query=11&query2#fragment");
}

BOOST_AUTO_TEST_CASE(print_compose_well_formed_with_basic_auth) { // NOLINT
    Uri u("scheme://username:password@www.example.com:8080");
    BOOST_TEST(u.compose() == "scheme://username:password@www.example.com:8080/");
}

BOOST_AUTO_TEST_CASE(print_compose_ill_formed_with_query) { // NOLINT
    Uri u("scheme://www.example.com?query=11");
    BOOST_TEST(u.compose() == "scheme://www.example.com/?query=11");
}

BOOST_AUTO_TEST_CASE(print_compose_ill_formed_with_fragment) { // NOLINT
    Uri u("scheme://www.example.com#fragment");
    BOOST_TEST(u.compose() == "scheme://www.example.com/#fragment");
}

BOOST_AUTO_TEST_CASE(print_compose_without_port) { // NOLINT
    Uri u("scheme://www.example.com");
    BOOST_TEST(u.compose() == "scheme://www.example.com/");
}

BOOST_AUTO_TEST_CASE(print_compose_without_scheme) { // NOLINT
    Uri u("www.example.com:8080");
    BOOST_TEST(u.compose() == "www.example.com:8080/");
}

BOOST_AUTO_TEST_CASE(print_compose_with_basic_auth) { // NOLINT
    Uri u("username:password@www.example.com:8080");
    BOOST_TEST(u.compose() == "username:password@www.example.com:8080/");
}

BOOST_AUTO_TEST_CASE(print_compose_special_scheme) { // NOLINT
    Uri u("https://www.example.com");
    BOOST_TEST(u.compose() == "https://www.example.com/");
}

BOOST_AUTO_TEST_CASE(print_compose_special_scheme_override) { // NOLINT
    Uri u("https://www.example.com:8443");
    BOOST_TEST(u.compose() == "https://www.example.com:8443/");
}

BOOST_AUTO_TEST_CASE(print_compose_ipv6) { // NOLINT
    Uri u("scheme://[2001:DB8::1]:8443");
    BOOST_TEST(u.compose() == "scheme://[2001:DB8::1]:8443/");
}

BOOST_AUTO_TEST_CASE(print_path_ext) { // NOLINT
    Uri u("scheme://www.example.com:8080/sample/path?query=11&query2#fragment");
    BOOST_TEST(u.path_ext() == "/sample/path?query=11&query2#fragment");
}

BOOST_AUTO_TEST_CASE(print_netloc_with_port) { // NOLINT
    Uri u("scheme://www.example.com:8080");
    BOOST_TEST(u.netloc() == "www.example.com:8080");
}

BOOST_AUTO_TEST_CASE(print_netloc_without_port) { // NOLINT
    Uri u("scheme://www.example.com");
    BOOST_TEST(u.netloc() == "www.example.com");
}

#pragma clang diagnostic pop