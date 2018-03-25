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

namespace server {
    namespace asio = boost::asio;
    namespace ip = boost::asio::ip;
    using nlohmann::json;
    using boost::asio::ip::tcp;
    using boost::asio::io_context;
    using boost::asio::ssl::context;

    extern struct exit_t {
        promise<void> http;
        promise<void> passive;
        promise<void> active;
    } exit;

    class Server {
    public:
        explicit Server(Options &options);

        Options *cfg();

        View *view();

        int start();

    private:
        unique_ptr<grpc::Server> _server;
        vector<thread> _handlers;
        io_context _ioc;
        Router _router;
        Options *_cfg;
        context _ctx;
        View _view;

        void http_thread();

        void passive_thread();

        void active_thread();

        void load_http_certificate(asio::ssl::context &ctx);
    };
}


#endif //SYNCAIDE_SERVER_H
