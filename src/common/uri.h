#ifndef SYNCAIDE_COMMON_ENDPOINT_H
#define SYNCAIDE_COMMON_ENDPOINT_H

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <exception>
#include <iostream>
#include <string>

using namespace std;

namespace common {
    using boost::lexical_cast;
    using boost::regex_match;
    using boost::smatch;
    using boost::regex;

    enum host_t { NONE, IPV4, IPV6, FQDN };

    static const unordered_map<string, uint16_t> special_scheme{ // NOLINT
        {"ftp", 21},
        {"http", 80},
        {"https", 443},
        {"ws", 80},
        {"wss", 443}
    };

    class Uri {
    private:
        string _scheme;
        string _username;
        string _password;
        string _host;
        uint16_t _port;
        host_t _type;
        string _path;
        string _query;
        string _fragment;

    public:
        Uri() : _port(0), _type(host_t::NONE) {};

        Uri(const Uri &uri) = default;

        explicit Uri(const string &source);

        Uri(
            const string &host,
            uint16_t port
        );

        Uri(
            const string &host,
            uint16_t port,
            const string &path
        );

        Uri(
            const string &host,
            uint16_t port,
            const string &path,
            const string &query
        );

        Uri(
            const string &host,
            uint16_t port,
            const string &path,
            const string &query,
            const string &fragment
        );

        Uri(
            const string &scheme,
            const string &host,
            uint16_t port
        );

        Uri(
            const string &scheme,
            const string &host,
            uint16_t port,
            const string &path
        );

        Uri(
            const string &scheme,
            const string &host,
            uint16_t port,
            const string &path,
            const string &query
        );

        Uri(
            const string &scheme,
            const string &host,
            uint16_t port,
            const string &path,
            const string &query,
            const string &fragment
        );

        const string &scheme() const;

        const string &username() const;

        const string &password() const;

        const string &host() const;

        const uint16_t port() const;

        const string &path() const;

        const string &query() const;

        const string &fragment() const;

        const string path_ext() const;

        const string compose() const;

        const string netloc() const;

        bool empty() const;

        bool is_ipv4() const;

        bool is_ipv6() const;

        bool is_fqdn() const;

        bool is_tls() const;

    private:
        void _authority(const string &source);
    };
}

#endif //SYNCAIDE_COMMON_ENDPOINT_H