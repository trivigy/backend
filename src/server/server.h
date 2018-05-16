#ifndef SYNCAIDE_SERVER_SERVER_H
#define SYNCAIDE_SERVER_SERVER_H

#include "server/options.h"
#include "server/peering.h"
#include "server/upstream.h"
#include "server/frontend.h"

#include <future>

using namespace std;

namespace server {
    extern struct exit_t {
        promise<void> peering;
        promise<void> upstream;
        promise<void> frontend;
    } exit;

    class Peering;

    class Upstream;

    class Frontend;

    class Server {
    private:
        Options &_cfg;
        vector<thread> _handlers;
        shared_ptr<Peering> _peering;
        shared_ptr<Upstream> _upstream;
        shared_ptr<Frontend> _frontend;

    public:
        explicit Server(Options &options);

        Options &cfg();

        shared_ptr<Peering> peering();

        shared_ptr<Upstream> upstream();

        shared_ptr<Frontend> frontend();

        int start();
    };
}


#endif //SYNCAIDE_SERVER_SERVER_H