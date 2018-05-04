#include "common/endpoint.h"

common::Endpoint::Endpoint(const string &source) : Endpoint() {
    regex expr(
        "(?:^(?<scheme>(?:[[:alpha:]][[:alnum:]+.-]*)*)(?::[/]{2}|[/]{2})?)"
        "(?:(?:(?<=[/]{2})|(?<=^))(?<authority>[^/?#\\s]*)(?!(?&authority)[/]{2}))"
    );

    smatch match;
    if (regex_match(source, match, expr)) {
        _scheme = match["scheme"];
        auto it = special_scheme.find(_scheme);
        if (it != special_scheme.end()) {
            _port = it->second;
        }

        _authority(match["authority"]);
        return;
    }
    throw std::invalid_argument("Failure parsing scheme and authority");
}

common::Endpoint::Endpoint(const string &host, uint16_t port) :
    Endpoint(host + ":" + to_string(port)) {}

common::Endpoint::Endpoint(
    const string &scheme,
    const string &host,
    uint16_t port
) : Endpoint(scheme + "://" + host + ":" + to_string(port)) {}

const string &common::Endpoint::scheme() const {
    return _scheme;
}

const string &common::Endpoint::host() const {
    return _host;
}

const uint16_t common::Endpoint::port() const {
    return _port;
}

void common::Endpoint::_authority(const string &source) {
    regex expr(
        "(?(DEFINE)"
        "(?<octet>(?<!\\d)(?:[1-9]?\\d|1\\d\\d|2(?:[0-4]\\d|5[0-5]))(?!\\d))"
        "(?<ipv4>(?:(?:(?&octet))[.]){3}(?:(?&octet)))"
        "(?<hextet>[[:xdigit:]]{1,4})"
        "(?<ipv6>(?:(?&hextet):){7,7}(?&hextet)|(?:(?&hextet):){1,7}:|(?:(?&hextet):){1,6}:(?&hextet)|(?:(?&hextet):){1,5}(?::(?&hextet)){1,2}|(?:(?&hextet):){1,4}(?::(?&hextet)){1,3}|(?:(?&hextet):){1,3}(?::(?&hextet)){1,4}|(?:(?&hextet):){1,2}(?::(?&hextet)){1,5}|(?&hextet):(?:(?::(?&hextet)){1,6})|:(?:(?::(?&hextet)){1,7}|:)|[fF][eE]80:(?::[[:xdigit:]]{0,4}){0,4}%[[:alnum:]]{1,}|::(?:[fF]{4}(?::0{1,4}){0,1}:){0,1}(?&ipv4)|(?:(?&hextet):){1,4}:(?&ipv4))"
        "(?<fqdn>(?=.{1,254}$)(?:(?=[a-z0-9-]{1,63}\\.)(xn--+)?[a-z0-9]+(-[a-z0-9]+)*\\.)+[a-z]{2,63}))"
        "^(?|(?<host>(?&ipv4))|\\[(?<host>(?&ipv6))\\]|(?<host>(?&fqdn)))"
        "(?::(?<port>(?:0|[1-9]\\d{0,3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])))?"
    );

    regex ipv4(
        "(?(DEFINE)"
        "(?<octet>(?<!\\d)(?:[1-9]?\\d|1\\d\\d|2(?:[0-4]\\d|5[0-5]))(?!\\d))"
        "(?<ipv4>(?:(?:(?&octet))[.]){3}(?:(?&octet))))"
        "^(?&ipv4)$"
    );

    regex ipv6(
        "(?(DEFINE)"
        "(?<octet>(?<!\\d)(?:[1-9]?\\d|1\\d\\d|2(?:[0-4]\\d|5[0-5]))(?!\\d))"
        "(?<ipv4>(?:(?:(?&octet))[.]){3}(?:(?&octet)))"
        "(?<hextet>[[:xdigit:]]{1,4})"
        "(?<ipv6>(?:(?&hextet):){7,7}(?&hextet)|(?:(?&hextet):){1,7}:|(?:(?&hextet):){1,6}:(?&hextet)|(?:(?&hextet):){1,5}(?::(?&hextet)){1,2}|(?:(?&hextet):){1,4}(?::(?&hextet)){1,3}|(?:(?&hextet):){1,3}(?::(?&hextet)){1,4}|(?:(?&hextet):){1,2}(?::(?&hextet)){1,5}|(?&hextet):(?:(?::(?&hextet)){1,6})|:(?:(?::(?&hextet)){1,7}|:)|[fF][eE]80:(?::[[:xdigit:]]{0,4}){0,4}%[[:alnum:]]{1,}|::(?:[fF]{4}(?::0{1,4}){0,1}:){0,1}(?&ipv4)|(?:(?&hextet):){1,4}:(?&ipv4)))"
        "^(?&ipv6)$"
    );

    regex fqdn(
        "(?(DEFINE)"
        "(?<fqdn>(?=.{1,254}$)(?:(?=[a-z0-9-]{1,63}\\.)(xn--+)?[a-z0-9]+(-[a-z0-9]+)*\\.)+[a-z]{2,63}))"
        "^(?&fqdn)$"
    );

    smatch match;
    if (regex_match(source, match, expr)) {
        _host = match["host"];
        if (regex_match(_host, ipv4)) _type = host_t::IPV4;
        if (regex_match(_host, ipv6)) _type = host_t::IPV6;
        if (regex_match(_host, fqdn)) _type = host_t::FQDN;

        if (match["port"].matched) {
            _port = lexical_cast<uint16_t>(match["port"]);
        }
        return;
    }
    throw std::invalid_argument("Failure parsing host and port");
}

string common::Endpoint::compose() {
    if (empty()) return string();

    string output;
    if (!_scheme.empty()) output += _scheme + "://";

    if (is_ipv6()) output += "[" + _host + "]";
    else output += _host;

    auto it = special_scheme.find(_scheme);
    if (it != special_scheme.end()) {
        if (_port != it->second) output += ":" + to_string(_port);
    } else {
        if (_port > 0) output += ":" + to_string(_port);
    }
    return output;
}

string common::Endpoint::netloc() {
    if (empty()) return string();

    if (_port > 0) return _host + ":" + to_string(_port);
    return _host;
}

bool common::Endpoint::empty() {
    return _type == host_t::NONE;
}

bool common::Endpoint::is_ipv4() {
    return _type == host_t::IPV4;
}

bool common::Endpoint::is_ipv6() {
    return _type == host_t::IPV6;
}

bool common::Endpoint::is_fqdn() {
    return _type == host_t::FQDN;
}