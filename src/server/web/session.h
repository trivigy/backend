#ifndef SYNCAIDE_SERVER_SESSION_H
#define SYNCAIDE_SERVER_SESSION_H

#include "server/ssl_stream.h"
#include "server/helper.h"
#include "common/uri.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/variant.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

namespace server {
    namespace web {
        namespace ssl = boost::asio::ssl;
        namespace http = boost::beast::http;
        using boost::system::error_code;
        using boost::beast::http::request;
        using boost::beast::http::response;
        using boost::beast::http::string_body;
        using boost::beast::flat_buffer;
        using boost::beast::string_param;
        using boost::asio::ip::tcp;
        using boost::asio::io_context;
        using boost::asio::error::eof;
        using boost::asio::async_connect;
        using boost::ignore_unused;
        using boost::variant;
        using common::Uri;

        class Session : public enable_shared_from_this<Session> {
            using plain_socket = tcp::socket;
            using ssl_socket = ssl_stream<tcp::socket>;
            using Socket = variant<plain_socket, ssl_socket>;
            using request_type = request<string_body>;
            using response_type = response<string_body>;

        private:
            bool _secured;
            Socket _socket;
            request_type _req;
            response_type _res;
            flat_buffer _buffer;
            tcp::resolver _resolver;
            function<void(response_type &)> _fn;

        public:
            template<typename Fn>
            explicit Session(
                io_context &ioc,
                ssl::context &ctx,
                request_type &req,
                Fn &&fn
            ) : _resolver(ioc),
                _socket(ssl_socket(plain_socket(ioc), ctx)),
                _secured(true),
                _req(req),
                _fn(fn) {}

            template<typename Fn>
            explicit Session(
                io_context &ioc,
                request_type &req,
                Fn &&fn
            ) : _resolver(ioc),
                _socket(plain_socket(ioc)),
                _secured(false),
                _req(req),
                _fn(fn) {}

            void run(Uri &uri);

            void on_resolve(error_code code, tcp::resolver::results_type results);

            void on_connect(error_code code);

            void on_handshake(error_code code);

            void on_write(error_code code, size_t bytes_transferred);

            void on_read(error_code code, size_t bytes_transferred);

            void on_shutdown(error_code code);
        };
    }
}

#endif //SYNCAIDE_SERVER_SESSION_H