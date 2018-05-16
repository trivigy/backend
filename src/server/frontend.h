#ifndef SYNCAIDE_SERVER_FRONTEND_H
#define SYNCAIDE_SERVER_FRONTEND_H

#include "server/router.h"
#include "server/http.h"
#include "server/server.h"
#include "server/listener.h"

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/io_context.hpp>
#include <string>

using namespace std;

namespace server {
    namespace ip = boost::asio::ip;
    using boost::asio::ssl::context;
    using boost::asio::io_context;

    class Server;

    class Frontend : public enable_shared_from_this<Frontend> {
    private:
        context _ctx;
        Router _router;
        io_context _ioc;
        Server &_server;
        vector<thread> _handlers;
//        map<string, shared_ptr<Miner>> _miners;

    public:
        explicit Frontend(Server &server);

        void start();

    private:
        void load_http_certificate(context &ctx);
    };
}

#endif //SYNCAIDE_SERVER_FRONTEND_H