#ifndef SYNCAIDE_SERVER_SESSION_H
#define SYNCAIDE_SERVER_SESSION_H

//#include "example/common/root_certificates.hpp"

#include "server/helper.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <functional>
#include <iostream>
#include <string>

using namespace std;

namespace server {
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
    
    class Session : public enable_shared_from_this<Session> {
    private:
        tcp::resolver _resolver;
        ssl::stream<tcp::socket> _stream;
        flat_buffer _buffer;
        request<string_body> _req;
        response<string_body> _res;

    public:
        explicit Session(io_context& ioc, ssl::context& ctx) :
            _resolver(ioc),
            _stream(ioc, ctx) {}

        void run(char const* host, char const* port, char const* target, int version) {
            if(! SSL_set_tlsext_host_name(_stream.native_handle(), host))
            {
                boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
                cerr << ec.message() << "\n";
                return;
            }

            // Set up an HTTP GET request message
            _req.version(version);
            _req.method(http::verb::get);
            _req.target(target);
            _req.set(http::field::host, string_param(host));
            _req.set(http::field::user_agent, string_param(BOOST_BEAST_VERSION_STRING));

            // Look up the domain name
            _resolver.async_resolve(
                host,
                port,
                bind(
                    &Session::on_resolve,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            );
        }

        void on_resolve(error_code code, tcp::resolver::results_type results) { // NOLINT
            if(code) return log("resolve", code);
            async_connect(
                _stream.next_layer(),
                results.begin(),
                results.end(),
                bind(
                    &Session::on_connect,
                    shared_from_this(),
                    placeholders::_1
                )
            );
        }

        void on_connect(error_code code) {
            if(code) return log("connect", code);
            _stream.async_handshake(
                ssl::stream_base::client,
                bind(
                    &Session::on_handshake,
                    shared_from_this(),
                    placeholders::_1
                )
            );
        }

//        void on_detect(error_code code, tribool secured);

        void on_handshake(error_code code) {
            if(code) return log("handshake", code);

            http::async_write(
                _stream,
                _req,
                bind(
                    &Session::on_write,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            );
        }

        void on_write(error_code code, size_t bytes_transferred) {
            ignore_unused(bytes_transferred);
            if(code) return log("write", code);

            http::async_read(_stream, _buffer, _res,
                bind(
                    &Session::on_read,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            );
        }

        void on_read(error_code code, size_t bytes_transferred) {
            ignore_unused(bytes_transferred);
            if(code) return log("read", code);

            // Write the message to standard out
            cout << _res << endl;

            _stream.async_shutdown(
                bind(
                    &Session::on_shutdown,
                    shared_from_this(),
                    placeholders::_1
                )
            );
        }

        void on_shutdown(error_code code) {
            if(code == eof) code.assign(0, code.category());
            if(code) return log("shutdown", code);
        }
    };
}

#endif //SYNCAIDE_SERVER_SESSION_H