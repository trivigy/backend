#ifndef SYNCAIDE_SERVER_PEERING_H
#define SYNCAIDE_SERVER_PEERING_H

#include "rpc/callers/members.h"
#include "rpc/services/members.h"
#include "rpc/services/miners.h"
#include "server/helper.h"
#include "server/server.h"
#include "view.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <grpc++/grpc++.h>
#include <functional>

using namespace std;
using namespace placeholders;

namespace server {
    using boost::asio::io_context;
    using boost::asio::steady_timer;

    class Server;

    class Peering : public enable_shared_from_this<Peering> {
    private:
        io_context _ioc;
        Server &_server;
        steady_timer _timer;
        shared_ptr<View> _view;
        vector<thread> _handlers;
        unique_ptr<grpc::Server> _rpc;

    public:
        explicit Peering(Server &server);

        void start();

        shared_ptr<View> view();

    private:
        void on_pulse(error_code code);
    };
}

#endif //SYNCAIDE_SERVER_PEERING_H