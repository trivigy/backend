#ifndef SYNCAIDE_SERVER_H
#define SYNCAIDE_SERVER_H

#include "server/response.h"
#include "server/options.h"
#include "server/listener.h"
#include "server/router.h"
#include "server/helper.h"
#include "server/http.h"
#include "server/web/client.h"
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
    using boost::asio::steady_timer;
    using boost::beast::http::request;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using boost::beast::http::verb;
    using boost::beast::http::field;

    extern struct exit_t {
        promise<void> peering;
        promise<void> upstream;
        promise<void> frontend;
    } exit;

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

    class Upstream : public enable_shared_from_this<Upstream> {
    private:
        io_context _ioc;
        Server &_server;
        vector<thread> _handlers;
        steady_timer _timer_info;
        steady_timer _timer_block_template;

    public:
        explicit Upstream(Server &server);

        void start();

    private:
        void check_info(error_code code);

        void check_block_template(error_code code);
    };

    class Frontend : public enable_shared_from_this<Frontend> {
    private:
        context _ctx;
        io_context _ioc;
        Server &_server;
        vector<thread> _handlers;
        shared_ptr<Router> _router;

    public:
        explicit Frontend(Server &server);

        void start();

    private:
        void load_http_certificate(asio::ssl::context &ctx);
    };

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


#endif //SYNCAIDE_SERVER_H
