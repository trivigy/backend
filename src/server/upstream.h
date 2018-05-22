#ifndef SYNCAIDE_SERVER_UPSTREAM_H
#define SYNCAIDE_SERVER_UPSTREAM_H

#include "server/web/client.h"
#include "server/server.h"
#include "server/helper.h"

#include <boost/beast/core.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <algorithm>

using namespace std;

namespace server {
    using boost::asio::io_context;
    using boost::asio::steady_timer;
    using boost::beast::http::request;
    using boost::beast::http::string_body;
    using boost::beast::http::verb;
    using boost::beast::http::field;

    class Server;

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
}

#endif //SYNCAIDE_SERVER_UPSTREAM_H
