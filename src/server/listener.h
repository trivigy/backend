#ifndef SYNCAIDE_SERVER_LISTENER_H
#define SYNCAIDE_SERVER_LISTENER_H

#include "server/helper.h"
#include "server/detector.h"
#include "server/router.h"

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

    class Listener : public enable_shared_from_this<Listener> {
    public:
        Listener(
            io_context &ioc,
            context &ctx,
            tcp::endpoint endp,
            Router &router
        );

        void run();

        void accept();

        void on_accept(error_code code);

    private:
        context &_ctx;
        tcp::acceptor _acceptor;
        tcp::socket _socket;
        Router &_router;
    };
}

#endif //SYNCAIDE_SERVER_LISTENER_H
