#ifndef SYNC_CLIENT_OPTIONS_H
#define SYNC_CLIENT_OPTIONS_H

#include "common/options.h"

#include <boost/format.hpp>

namespace client {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;

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

#endif //SYNC_CLIENT_OPTIONS_H
