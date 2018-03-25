#ifndef SYNCAIDE_CLIENT_H
#define SYNCAIDE_CLIENT_H

#include "client/options.h"
#include "rpc/callers/members.h"

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
    public:
        explicit Client(Options &options);

        Options *cfg();

        int start();

    private:
        Options *_cfg;

        int members_gossip();

        int members_list();

        int members_status();
    };
}

#endif //SYNCAIDE_CLIENT_H