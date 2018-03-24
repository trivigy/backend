#ifndef SYNCAIDE_CLIENT_OPTIONS_H
#define SYNCAIDE_CLIENT_OPTIONS_H

#include "common/options.h"

#include <boost/format.hpp>
#include <boost/dll.hpp>

namespace client {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;
    using boost::dll::program_location;

    class Options final : common::Options {
    public:
        const struct {
            const struct {
                const string host = "127.0.0.1:8847"; // NOLINT
            } network;
        } defaults;

        string cmd;

        struct {
            string host;
        } network;

        struct {
            string cmd;
        } members;

        bool parse(int argc, const char **argv) override;

    private:
        void on_host(string host);
    };
}

#endif //SYNCAIDE_CLIENT_OPTIONS_H
