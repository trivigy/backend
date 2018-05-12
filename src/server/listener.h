#ifndef SYNCAIDE_SERVER_LISTENER_H
#define SYNCAIDE_SERVER_LISTENER_H

#include "server/helper.h"
#include "server/handoff.h"
#include "server/router.h"
#include "server/server.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <string>

using namespace std;

namespace server {
    namespace ssl = boost::asio::ssl;
    using boost::system::error_code;
    using boost::asio::socket_base;
    using boost::asio::io_context;
    using boost::asio::ssl::context;

    class Server;

    class Listener : public enable_shared_from_this<Listener> {
    private:
        context &_ctx;
        Server &_server;
        tcp::socket _socket;
        tcp::acceptor _acceptor;
        shared_ptr<Router> _router;

    public:
        Listener(
            Server &server,
            io_context &ioc,
            context &ctx,
            shared_ptr<Router> router,
            tcp::endpoint &endp
        );

        void run();

        void on_accept(error_code code);
    };
}

#endif //SYNCAIDE_SERVER_LISTENER_H
