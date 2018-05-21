#ifndef SYNCAIDE_CLIENT_OPTIONS_H
#define SYNCAIDE_CLIENT_OPTIONS_H

#include "common/options.h"
#include "common/uri.h"

#include <boost/format.hpp>
#include <boost/dll.hpp>

namespace client {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;
    using boost::dll::program_location;
    using common::Uri;

    class Options final : common::Options {
    public:
        const struct {
            const struct {
                const Uri host{"127.0.0.1", 8847};
            } network;
        } defaults;

        string cmd;

        struct {
            Uri host;
        } network;

        struct {
            string cmd;
        } members;

        struct {
            string cmd;
        } miners;

        bool parse(int argc, const char **argv) override;

    private:
        void on_host(string uri);
    };
}

#endif //SYNCAIDE_CLIENT_OPTIONS_H
