#include "common/uri.h"

common::Uri::Uri(const string &source) : Uri() {
    regex expr(
        "(?:^(?<scheme>(?:[[:alpha:]][[:alnum:]+.-]*)*)(?::[/]{2}|[/]{2})?)"
        "(?:(?:(?<=[/]{2})|(?<=^))(?<authority>[^/?#\\s]*)(?!(?&authority)[/]{2}))"
        "(?<path>[^?#\\s]*)"
        "(?:\\?(?<query>[^#\\s]*))?"
        "(?:#(?<fragment>.*))?"
    );

    smatch match;
    if (regex_match(source, match, expr)) {
        _scheme = match["scheme"];
        auto it = special_scheme.find(_scheme);
        if (it != special_scheme.end()) {
            _port = it->second;
        }

        _authority(match["authority"]);
        _path = match["path"];
        if (_path.empty()) _path = "/";
        if (match["query"].matched) _query = match["query"];
        if (match["fragment"].matched) _fragment = match["fragment"];
        return;
    }
    throw invalid_argument("Failure parsing uri.");
}

common::Uri::Uri(const string &host, uint16_t port) :
    Uri(host + ":" + to_string(port)) {}

common::Uri::Uri(
    const string &scheme,
    const string &host,
    uint16_t port
) : Uri(scheme + "://" + host + ":" + to_string(port)) {}

const string &common::Uri::scheme() const {
    return _scheme;
}

const string &common::Uri::username() const {
    return _username;
}

const string &common::Uri::password() const {
    return _password;
}

const string &common::Uri::host() const {
    return _host;
}

const uint16_t common::Uri::port() const {
    return _port;
}

const string &common::Uri::path() const {
    return _path;
}

const string common::Uri::path_ext() const {
    string output;
    output += _path;
    if (!_query.empty()) output += "?" + _query;
    if (!_fragment.empty()) output += "#" + _fragment;
    return output;
}

const string &common::Uri::query() const {
    return _query;
}

const string &common::Uri::fragment() const {
    return _fragment;
}

void common::Uri::_authority(const string &source) {
    regex expr(
        "(?(DEFINE)"
        "(?<octet>(?<!\\d)(?:[1-9]?\\d|1\\d\\d|2(?:[0-4]\\d|5[0-5]))(?!\\d))"
        "(?<ipv4>(?:(?:(?&octet))[.]){3}(?:(?&octet)))"
        "(?<hextet>[[:xdigit:]]{1,4})"
        "(?<ipv6>(?:(?&hextet):){7,7}(?&hextet)|(?:(?&hextet):){1,7}:|(?:(?&hextet):){1,6}:(?&hextet)|(?:(?&hextet):){1,5}(?::(?&hextet)){1,2}|(?:(?&hextet):){1,4}(?::(?&hextet)){1,3}|(?:(?&hextet):){1,3}(?::(?&hextet)){1,4}|(?:(?&hextet):){1,2}(?::(?&hextet)){1,5}|(?&hextet):(?:(?::(?&hextet)){1,6})|:(?:(?::(?&hextet)){1,7}|:)|[fF][eE]80:(?::[[:xdigit:]]{0,4}){0,4}%[[:alnum:]]{1,}|::(?:[fF]{4}(?::0{1,4}){0,1}:){0,1}(?&ipv4)|(?:(?&hextet):){1,4}:(?&ipv4))"
        "(?<fqdn>(?:localhost)|(?=.{1,254}$)(?:(?=[a-z0-9-]{1,63}\\.)(xn--+)?[a-z0-9]+(-[a-z0-9]+)*\\.)+[a-z]{2,63}))"
        "^(?:(?<username>[^@:\\s]*)(?::(?<password>[^@\\s]*))?@)?"
        "(?|(?<host>(?&ipv4))|\\[(?<host>(?&ipv6))\\]|(?<host>(?&fqdn)))"
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
        "(?<fqdn>(?:localhost)|(?=.{1,254}$)(?:(?=[a-z0-9-]{1,63}\\.)(xn--+)?[a-z0-9]+(-[a-z0-9]+)*\\.)+[a-z]{2,63}))"
        "^(?&fqdn)$"
    );

    smatch match;
    if (regex_match(source, match, expr)) {
        _host = match["host"];
        if (regex_match(_host, ipv4)) _type = host_t::IPV4;
        if (regex_match(_host, ipv6)) _type = host_t::IPV6;
        if (regex_match(_host, fqdn)) _type = host_t::FQDN;

        if (match["username"].matched) _username = match["username"];
        if (match["password"].matched) _password = match["password"];
        if (match["port"].matched)
            _port = lexical_cast<uint16_t>(match["port"]);
        return;
    }
    throw invalid_argument("Failure parsing authority.");
}

const string common::Uri::compose() const {
    if (empty()) return string();

    string output;
    if (!_scheme.empty()) output += _scheme + "://";

    if (!_username.empty()) {
        output += _username;
        if (!_password.empty()) output += ":" + _password;
        output += "@";
    }

    if (is_ipv6()) output += "[" + _host + "]";
    else output += _host;

    auto it = special_scheme.find(_scheme);
    if (it != special_scheme.end()) {
        if (_port != it->second) output += ":" + to_string(_port);
    } else {
        if (_port > 0) output += ":" + to_string(_port);
    }

    output += _path;
    if (!_query.empty()) output += "?" + _query;
    if (!_fragment.empty()) output += "#" + _fragment;
    return output;
}

const string common::Uri::netloc() const {
    if (empty()) return string();

    if (_port > 0) return _host + ":" + to_string(_port);
    return _host;
}

bool common::Uri::empty() const {
    return _type == host_t::NONE;
}

bool common::Uri::is_ipv4() const {
    return _type == host_t::IPV4;
}

bool common::Uri::is_ipv6() const {
    return _type == host_t::IPV6;
}

bool common::Uri::is_fqdn() const {
    return _type == host_t::FQDN;
}