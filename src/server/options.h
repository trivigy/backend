#ifndef SYNCAIDE_SERVER_OPTIONS_H
#define SYNCAIDE_SERVER_OPTIONS_H

#include "common/options.h"
#include "common/endpoint.h"

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
    using common::Endpoint;
    using nlohmann::json;

    class Options final : common::Options {
    public:
        const struct {
            const struct {
                const Endpoint bind{"127.0.0.1", 8847};
                const Endpoint upstream{"127.0.0.1", 18081};
                const Endpoint frontend{"127.0.0.1", 8080};
                const unsigned int threads = thread::hardware_concurrency();
            } network;
        } defaults;


        struct {
            Endpoint advertise;
            Endpoint bind;
            vector<Endpoint> joins;
            vector<Endpoint> upstreams;
            Endpoint frontend;

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
        void on_advertise(string endpoint);

        void on_bind(string endpoint);

        void on_joins(vector<string> endpoints);

        void on_upstreams(vector<string> endpoints);

        void on_frontend(string endpoint);

        void on_threads(int threads);
    };
}

#endif //SYNCAIDE_SERVER_OPTIONS_H
