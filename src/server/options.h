#ifndef SYNC_SERVER_OPTIONS_H
#define SYNC_SERVER_OPTIONS_H

#include "common/options.h"

#include <nlohmann/json.hpp>

namespace server {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;
    using json = nlohmann::json;

    class Options final : common::Options {
    public:
        const struct {
            const struct {
                const string bind = "127.0.0.1:8847"; // NOLINT
            } network;
        } defaults;

        struct {
            string advertise;
            string bind;
            vector<string> joins;
        } network;

        struct {
            int c = 30;
            int H = c / 2;
            int S = 0;
        } members;

        bool parse(int argc, const char **argv) override;

    private:
        void on_bind(string bind);

        void on_joins(vector<string> joins);
    };
}

#endif //SYNC_SERVER_OPTIONS_H
