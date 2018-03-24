#ifndef SYNCAIDE_SERVER_DISPATCHER_H
#define SYNCAIDE_SERVER_DISPATCHER_H

#include "server/response.h"

#include <boost/beast/core.hpp>
#include <fmt/format.h>
#include <regex>
#include <iostream>

using namespace std;

namespace server {
    using boost::beast::http::status;
    using boost::beast::http::request;
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
    public:
        template<typename Fn, typename Tuple>
        Rule(Fn &&fn, const string &pattern, Tuple) :
            _regex(pattern),
            _fn([&fn](smatch &match, request<string_body> &req) {
                Tuple params;
                auto it = ++match.begin();
                tuple_for_each(params, [&it](auto &v) {
                    istringstream iss(*it++);
                    iss >> v;
                });
                return apply(fn, move(tuple_cat(tie(req), params)));
            }) {}

        response<string_body> dispatch(request<string_body> &req) const {
            const string &route = req.target().to_string();

            smatch match;
            if (!regex_match(route.begin(), route.end(), match, _regex)) {
                return response<string_body>(status::unknown, req.version());
            }
            return _fn(match, req);
        }

    private:
        regex _regex;
        function<
            response<string_body>(
                smatch &match,
                request<string_body> &req
            )
        > _fn;
    };

    class Router {
    public:
        template<typename Fn, typename... Args>
        void add(Fn &&fn, const string &pattern, Args ... args) {
            _rules.emplace_back(
                forward<Fn>(fn),
                fmt::format(pattern, args.pattern...),
                tuple_cat(args.type...)
            );
        }

        response<string_body> dispatch(request<string_body> &req) const {
            vector<response<string_body>> resps;
            for_each(_rules.begin(), _rules.end(),
                [&resps, &req](const Rule &rule) {
                    resps.emplace_back(rule.dispatch(req));
                }
            );

            auto const it = find_if(resps.begin(), resps.end(),
                [](auto &resp) {
                    return resp.result() != status::unknown;
                }
            );

            if (it != resps.end()) {
                return *it;
            }
            return Response::not_found(req);
        }

    private:
        vector<Rule> _rules;
    };

}

#endif //SYNCAIDE_SERVER_DISPATCHER_H
