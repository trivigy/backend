#ifndef SYNCAIDE_SERVER_OPTIONS_H
#define SYNCAIDE_SERVER_OPTIONS_H

#include "common/options.h"
#include "common/uri.h"

#include <nlohmann/json.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/dll.hpp>
#include <thread>

using namespace std;

namespace server {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;
    namespace fs = boost::filesystem;
    using boost::dll::program_location;
    using common::Uri;
    using nlohmann::json;

    class Options final : common::Options {
    public:
        const struct {
            const struct {
                const Uri bind{"127.0.0.1", 8847};
                const Uri upstream{"127.0.0.1", 18081};
                const Uri frontend{"127.0.0.1", 8080};
                const unsigned int threads = thread::hardware_concurrency();
            } network;
        } defaults;


        struct {
            Uri advertise;
            Uri bind;
            vector<Uri> joins;
            Uri upstream;
            Uri frontend;

            unsigned int threads;
        } network;

        struct {
            int c = 30;
            int H = c / 2;
            int S = 0;
        } members;

    public:
        bool parse(int argc, const char **argv) override;

    private:
        void on_advertise(string uri);

        void on_bind(string uri);

        void on_joins(vector<string> uris);

        void on_upstream(string uri);

        void on_frontend(string uri);

        void on_threads(int threads);
    };
}

#endif //SYNCAIDE_SERVER_OPTIONS_H
