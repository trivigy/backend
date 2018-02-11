#ifndef SYNC_SERVER_H
#define SYNC_SERVER_H

#include "server/options.h"
#include "rpc/members.h"
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
    using json = nlohmann::json;
    using tcp = boost::asio::ip::tcp;
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;
    namespace websocket = boost::beast::websocket;

    extern struct exit_t {
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
        Options *_cfg;
        View _view;

        void http_thread();

        void passive_thread();

        void active_thread();
    };
}


#endif //SYNC_SERVER_H
