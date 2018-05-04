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

    class Endpoint {
    private:
        string _scheme;
        string _host;
        uint16_t _port;
        host_t _type;

    public:
        Endpoint() : _port(0), _type(host_t::NONE) {};

        Endpoint(const Endpoint &uri) = default;

        explicit Endpoint(const string &source);

        Endpoint(const string &host, uint16_t port);

        Endpoint(const string &scheme, const string &host, uint16_t port);

        const string &scheme() const;

        const string &host() const;

        const uint16_t port() const;

        string compose();

        string netloc();

        bool empty();

        bool is_ipv4();

        bool is_ipv6();

        bool is_fqdn();

    private:
        void _authority(const string &source);
    };
}

#endif //SYNCAIDE_COMMON_ENDPOINT_H