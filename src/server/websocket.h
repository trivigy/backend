#ifndef SYNC_SERVER_WEBSOCKET_H
#define SYNC_SERVER_WEBSOCKET_H

#include "server/helper.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/logic/tribool.hpp>
#include <functional>

using namespace std;

namespace server {
    namespace asio = boost::asio;
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;
    namespace websocket = boost::beast::websocket;
    using boost::system::error_code;
    using boost::beast::multi_buffer;
    using boost::asio::bind_executor;
    using boost::asio::steady_timer;
    using boost::asio::ssl::context;
    using boost::asio::io_context;
    using boost::asio::ip::tcp;
    using boost::asio::strand;
    using boost::asio::error::operation_aborted;
    using boost::tribool;

    class Websocket : public enable_shared_from_this<Websocket> {
    public:
        explicit Websocket(
            tcp::socket socket,
            tribool secured,
            context &ctx
        );

        void run(http::request<http::string_body> &&req);

        void accept(http::request<http::string_body> &&req);

        void read();

        void timeout();

        void on_timer(error_code code);

        void on_accept(error_code code);

        void on_conclude(error_code code);

        void on_read(error_code code, size_t bytes_transferred);

        void on_write(error_code code, size_t bytes_transferred);

    private:
        multi_buffer _buffer;
        steady_timer _timer;
        bool _close = false;
        bool _eof = false;
        context &_ctx;

    protected:
        tcp::socket _socket;
        websocket::stream<tcp::socket &> _plain;
        websocket::stream<ssl::stream<tcp::socket &>> _secure;
        strand<io_context::executor_type> _strand;
        tribool _secured;
    };
}

#endif //SYNC_SERVER_WEBSOCKET_H
