#ifndef SYNCAIDE_SERVER_DISPATCHER_H
#define SYNCAIDE_SERVER_DISPATCHER_H

#include "server/response.h"

#include <boost/beast/core.hpp>
#include <fmt/format.h>
#include <iostream>
#include <regex>

using namespace std;

namespace server {
    using boost::beast::http::status;
    using boost::beast::http::response;
    using boost::beast::http::string_body;

    namespace params {
        struct integer {
            const string pattern = "([[:digit:]]+)";
            const tuple<int> type;
        };

        struct string {
            const std::string pattern = "([_\\-[:alnum:]]+)";
            const tuple<std::string> type;
        };
    }

    template<typename Tuple, typename Fn, size_t index = 0>
    inline enable_if_t<index == tuple_size_v<Tuple>>
    tuple_for_each(Tuple &, Fn &&) {}

    template<typename Tuple, typename Fn, size_t index = 0>
    inline enable_if_t<index < tuple_size_v<Tuple>>
    tuple_for_each(Tuple &t, Fn &&f) {
        f(std::get<index>(t));
        tuple_for_each<Tuple, Fn, index + 1>(t, forward<Fn>(f));
    }

    class Rule {
    private:
        regex _regex;
        function<int(std::smatch &match, void *server, void *req)> _fn;

    public:
        template<typename Fn, typename Tuple>
        Rule(Fn &&fn, const string &pattern, Tuple) :
            _regex(pattern),
            _fn([&fn](smatch &match, void *server, void *req) {
                Tuple params;
                auto it = ++match.begin();
                tuple_for_each(params, [&it](auto &v) {
                    istringstream iss(*it++);
                    iss >> v;
                });
                return apply(fn, tuple_cat(tie(server), tie(req), params));
            }) {}

        template<typename Server, typename Request>
        int dispatch(Server &server, Request &req) const {
            smatch match;
            const string &route = req.target().to_string();
            if (regex_match(route.begin(), route.end(), match, _regex)) {
                return _fn(match, &server, &req);
            }
            return 0;
        }
    };

    class Router {
    private:
        vector<Rule> _rules;

    public:
        template<typename Fn, typename... Args>
        void add(Fn &&fn, const string &pattern, Args ... args) {
            _rules.emplace_back(
                forward<Fn>(fn),
                fmt::format(pattern, args.pattern...),
                tuple_cat(args.type...)
            );
        }

        template<typename Server, typename Request>
        int dispatch(Server &server, Request &req) const {
            vector<int> resps;
            for_each(_rules.begin(), _rules.end(),
                [&resps, &server, &req](const Rule &rule) {
                    resps.emplace_back(rule.dispatch(server, req));
                }
            );

            const auto it = find_if(resps.begin(), resps.end(),
                [](auto &resp) { return resp != 0; }
            );

            if (it != resps.end()) {
                return *it;
            }
            return 404;
        }
    };

}

#endif //SYNCAIDE_SERVER_DISPATCHER_H
