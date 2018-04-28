#ifndef SYNCAIDE_SERVER_H
#define SYNCAIDE_SERVER_H

#include "server/response.h"
#include "server/options.h"
#include "server/listener.h"
#include "server/router.h"
#include "server/http.h"
#include "rpc/callers/members.h"
#include "rpc/services/members.h"
#include "view.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <nlohmann/json.hpp>
#include <grpc++/grpc++.h>
#include <iostream>
#include <thread>
#include <cstdlib>
#include <csignal>
#include <future>

using namespace std;
using namespace placeholders;

namespace server {
    namespace asio = boost::asio;
    namespace ip = boost::asio::ip;
    using nlohmann::json;
    using boost::asio::ip::tcp;
    using boost::asio::io_context;
    using boost::asio::ssl::context;

    extern struct exit_t {
        promise<void> peering;
        promise<void> upstream;
        promise<void> frontend;
    } exit;

    class Server;

    class Peering : public enable_shared_from_this<Peering> {
    public:
        explicit Peering(shared_ptr<Server> server);

        void start();

        shared_ptr<View> view();

    private:
        io_context _ioc;
        steady_timer _timer;
        shared_ptr<View> _view;
        vector<thread> _handlers;
        shared_ptr<Server> _server;
        unique_ptr<grpc::Server> _rpc;

        void on_pulse(error_code code);
    };

    class Upstream : public enable_shared_from_this<Upstream> {
    public:
        explicit Upstream(shared_ptr<Server> server);

        void start();

    private:
        shared_ptr<Server> _server;
    };

    class Frontend : public enable_shared_from_this<Frontend> {
    public:
        explicit Frontend(shared_ptr<Server> server);

        void start();

    private:
        shared_ptr<Server> _server;
        shared_ptr<Router> _router;
        vector<thread> _handlers;
        io_context _ioc;
        context _ctx;

        void load_http_certificate(asio::ssl::context &ctx);
    };

    class Server : public enable_shared_from_this<Server> {
    public:
        static shared_ptr<Server> create(shared_ptr<Options> options);

        shared_ptr<Options> cfg();

        shared_ptr<Peering> peering();

        shared_ptr<Upstream> upstream();

        shared_ptr<Frontend> frontend();

        int start();

    private:
        explicit Server(shared_ptr<Options> options);

        vector<thread> _handlers;
        shared_ptr<Options> _cfg;
        shared_ptr<Peering> _peering;
        shared_ptr<Upstream> _upstream;
        shared_ptr<Frontend> _frontend;
    };
}


#endif //SYNCAIDE_SERVER_H
