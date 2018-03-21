#ifndef SYNCAIDE_SERVER_WEBSOCKET_H
#define SYNCAIDE_SERVER_WEBSOCKET_H

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
#include <nlohmann/json.hpp>
#include <functional>

using namespace std;

namespace server {
    namespace asio = boost::asio;
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;
    namespace websocket = boost::beast::websocket;
    using nlohmann::json;
    using boost::system::error_code;
    using boost::beast::http::request;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
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
        using request_type = request<string_body>;
        using response_type = response<string_body>;

    public:
        explicit Websocket(
            tcp::socket socket,
            tribool secured,
            context &ctx,
            json &&params
        );

        void run(request<string_body> &&req);

        void accept(request<string_body> &&req);

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
        json _params;

    protected:
        tcp::socket _socket;
        websocket::stream<tcp::socket &> _plain;
        websocket::stream<ssl::stream<tcp::socket &>> _secure;
        strand<io_context::executor_type> _strand;
        tribool _secured;
    };
}

#endif //SYNCAIDE_SERVER_WEBSOCKET_H
