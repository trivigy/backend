#ifndef SYNCAIDE_CLIENT_H
#define SYNCAIDE_CLIENT_H

#include "client/options.h"
#include "rpc/callers/peers.h"
#include "rpc/callers/miners.h"

#include <nlohmann/json.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <csignal>
#include <string>

using namespace std;

namespace client {
    namespace po = boost::program_options;
    namespace cls = po::command_line_style;
    using json = nlohmann::json;

    class Client {
        class Peers {
        private:
            Client &_self;

        public:
            explicit Peers(Client &self) : _self(self) {}

            int list();
        };

        class Miners {
        private:
            Client &_self;

        public:
            explicit Miners(Client &self) : _self(self) {}

            int list();
        };

    public:
        Peers peers;
        Miners miners;

    private:
        Options &_cfg;

    public:
        explicit Client(Options &options);

        Options &cfg();

        int start();
    };
}

#endif //SYNCAIDE_CLIENT_H