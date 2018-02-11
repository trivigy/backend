#ifndef SYNC_OPTIONS_H
#define SYNC_OPTIONS_H

#include <boost/program_options/options_description.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <regex>

using namespace std;

namespace common {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;

    class Options {
    public:
        virtual bool parse(int argc, const char **argv) = 0;

    protected:
        string basename(const string &path);

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