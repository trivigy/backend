#ifndef SYNCAIDE_SERVER_WEBSOCKET_H
#define SYNCAIDE_SERVER_WEBSOCKET_H

#include "server/helper.h"
#include "server/ssl_stream.h"
#include "protos/message.pb.h"
#include "server/server.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/variant.hpp>
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
    using boost::beast::buffers_to_string;
    using boost::beast::http::request;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using boost::beast::http::field;
    using boost::beast::multi_buffer;
    using boost::beast::string_param;
    using boost::asio::bind_executor;
    using boost::asio::steady_timer;
    using boost::asio::ssl::context;
    using boost::asio::io_context;
    using boost::asio::ip::tcp;
    using boost::asio::strand;
    using boost::asio::const_buffer;
    using boost::asio::const_buffers_1;
    using boost::asio::error::operation_aborted;
    using boost::ignore_unused;
    using boost::variant;
    using boost::tribool;

    class Server;

    class Miner : public enable_shared_from_this<Miner> {
        using plain_socket = websocket::stream<tcp::socket>;
        using ssl_socket = websocket::stream<ssl_stream<tcp::socket>>;
        using Socket = variant<plain_socket, ssl_socket>;
        using request_type = request<string_body>;
        using response_type = response<string_body>;

    private:
        Server &_server;
        multi_buffer _buffer;
        steady_timer _timer;
        bool _close = false;
        bool _eof = false;
        context &_ctx;
        string _uid;

    protected:
        Socket _socket;
        strand<io_context::executor_type> _strand;
        tribool _secured;

    public:
        explicit Miner(
            Server &server,
            context &ctx,
            ssl_stream<tcp::socket> socket,
            tribool secured,
            const string &uid
        );

        explicit Miner(
            Server &server,
            context &ctx,
            tcp::socket socket,
            tribool secured,
            const string &uid
        );

        const string &uid();

        void run(request<string_body> &&req);

    private:

        void accept(request<string_body> &&req);

        void on_accept(error_code code);

        void read();

        void on_read(error_code code, size_t bytes_transferred);

        void write(const BOOST_ASIO_CONST_BUFFER &buffer);

        void on_write(error_code code, size_t bytes_transferred);

        void on_timer(error_code code);

        void timeout();

        void on_conclude(error_code code);
    };
}

#endif //SYNCAIDE_SERVER_WEBSOCKET_H
