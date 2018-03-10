#ifndef SYNC_SERVER_DISPATCHER_H
#define SYNC_SERVER_DISPATCHER_H

#include <fmt/format.h>
#include <regex>
#include <iostream>

using namespace std;

namespace server {
    namespace params {
        struct integer {
            const string pattern = "([[:digit:]]+)";
            const tuple<int> type;
        };

        struct string {
            const std::string pattern = "([_[:alnum:]]+)";
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
            _pattern(pattern),
            _fn([&fn](smatch &match) {
                Tuple args;
                auto it = ++match.begin();
                tuple_for_each(args, [&it](auto &v) {
                    istringstream iss(*it++);
                    iss >> v;
                });
                apply(fn, move(args));
            }) {}

        bool dispatch(const string &route) const {
            smatch match;
            if (!regex_match(route.begin(), route.end(), match, _regex)) {
                return false;
            }
            _fn(match);
            return true;
        }

    private:
        regex _regex;
        string _pattern;
        function<void(std::smatch &match)> _fn;
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

        bool dispatch(const string &route) const {
            return any_of(begin(_rules), end(_rules),
                [&route](const Rule &rule) {
                    return rule.dispatch(route);
                }
            );
        }

    private:
        vector<Rule> _rules;
    };

}

#endif //SYNC_SERVER_DISPATCHER_H
