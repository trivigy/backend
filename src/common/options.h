#ifndef SYNC_OPTIONS_H
#define SYNC_OPTIONS_H

#include <boost/program_options/options_description.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <regex>
#include <tuple>

using namespace std;

namespace common {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;

    class Options {
    public:
        const struct {
            const string netloc = "^(?:(?<ipv4>(?:(?:(?:[0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}(?:[0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])))|\\[(?<ipv6>(?:s*(?:(?:(?:[0-9A-Fa-f]{1,4}:){7}(?:[0-9A-Fa-f]{1,4}|:))|(?:(?:[0-9A-Fa-f]{1,4}:){6}(?::[0-9A-Fa-f]{1,4}|(?:(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)(?:.(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)){3})|:))|(?:(?:[0-9A-Fa-f]{1,4}:){5}(?:(?:(?::[0-9A-Fa-f]{1,4}){1,2})|:(?:(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)(?:.(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)){3})|:))|(?:(?:[0-9A-Fa-f]{1,4}:){4}(?:(?:(?::[0-9A-Fa-f]{1,4}){1,3})|(?:(?::[0-9A-Fa-f]{1,4})?:(?:(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)(?:.(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(?:(?:[0-9A-Fa-f]{1,4}:){3}(?:(?:(?::[0-9A-Fa-f]{1,4}){1,4})|(?:(?::[0-9A-Fa-f]{1,4}){0,2}:(?:(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)(?:.(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(?:(?:[0-9A-Fa-f]{1,4}:){2}(?:(?:(?::[0-9A-Fa-f]{1,4}){1,5})|(?:(?::[0-9A-Fa-f]{1,4}){0,3}:(?:(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)(?:.(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(?:(?:[0-9A-Fa-f]{1,4}:){1}(?:(?:(?::[0-9A-Fa-f]{1,4}){1,6})|(?:(?::[0-9A-Fa-f]{1,4}){0,4}:(?:(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)(?:.(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:))|(?::(?:(?:(?::[0-9A-Fa-f]{1,4}){1,7})|(?:(?::[0-9A-Fa-f]{1,4}){0,5}:(?:(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)(?:.(?:25[0-5]|2[0-4]d|1dd|[1-9]?d)){3}))|:)))(?:%.+)?s*))\\]|(\\g<ipv6>))(?::(?<port>[1-9][0-9]{4}|[1-9][0-9]{3}|[1-9][0-9]{2}|[1-9][0-9]|[1-9])?)?$"; // NOLINT
        } exprs;

        virtual bool parse(int argc, const char **argv) = 0;

    protected:
        string usage(
            const string &program_name,
            string description,
            vector<po::options_description> &descriptors,
            vector<po::positional_options_description> &positions,
            vector<po::variables_map> &maps,
            po::options_description commands = po::options_description()
        );
    };
}

#endif //SYNC_OPTIONS_H