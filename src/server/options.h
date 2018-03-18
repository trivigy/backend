#ifndef SYNCAIDE_SERVER_OPTIONS_H
#define SYNCAIDE_SERVER_OPTIONS_H

#include "common/options.h"

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
    using nlohmann::json;

    class Options final : common::Options {
    public:
        const struct {
            const struct {
                const struct bind_t {
                    const string address = "127.0.0.1"; // NOLINT
                    const unsigned short port = 8847;

                    const string netloc() const {
                        return address + ":" + to_string(port);
                    }
                } bind;
                const struct http_t {
                    const string address = "127.0.0.1"; // NOLINT
                    const unsigned short port = 8080;

                    const string netloc() const {
                        return address + ":" + to_string(port);
                    }
                } http;
                const unsigned int threads = thread::hardware_concurrency();
            } network;

            const struct {
                const string http_dir = "../share/sync"; // NOLINT
            } system;
        } defaults;

        struct {
            string advertise;
            string bind;

            struct http_t {
                string address;
                unsigned short port;

                const string netloc() const {
                    return address + ":" + to_string(port);
                }
            } http;

            vector<string> joins;
            unsigned int threads;
        } network;

        struct {
            string http_dir;
        } system;

        struct {
            int c = 30;
            int H = c / 2;
            int S = 0;
        } members;

        bool parse(int argc, const char **argv) override;

    private:
        void on_bind(string netloc);

        void on_http(string netloc);

        void on_http_dir(string dir);

        void on_joins(vector<string> joins);

        void on_threads(int threads);
    };
}

#endif //SYNCAIDE_SERVER_OPTIONS_H
