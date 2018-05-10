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

            void run(Uri &uri) {
                if (_secured) {
                    if (!SSL_set_tlsext_host_name(
                        boost::get<ssl_socket>(_socket).native_handle(),
                        uri.host().c_str()
                    )) {
                        error_code code{
                            static_cast<int>(::ERR_get_error()),
                            boost::asio::error::get_ssl_category()
                        };
                        log("shutdown", code);
                        return;
                    }
                }

                _resolver.async_resolve(
                    uri.host().c_str(),
                    to_string(uri.port()).c_str(),
                    bind(
                        &Session::on_resolve,
                        shared_from_this(),
                        placeholders::_1,
                        placeholders::_2
                    )
                );
            }

            void on_resolve(error_code code,
                tcp::resolver::results_type results) { // NOLINT
                if (code) return log("resolve", code);

                if (_secured) {
                    async_connect(
                        boost::get<ssl_socket>(_socket).next_layer(),
                        results.begin(),
                        results.end(),
                        bind(
                            &Session::on_connect,
                            shared_from_this(),
                            placeholders::_1
                        )
                    );
                } else {
                    async_connect(
                        boost::get<plain_socket>(_socket),
                        results.begin(),
                        results.end(),
                        bind(
                            &Session::on_connect,
                            shared_from_this(),
                            placeholders::_1));
                }
            }

            void on_connect(error_code code) {
                if (code) return log("connect", code);
                if (_secured) {
                    boost::get<ssl_socket>(_socket).async_handshake(
                        ssl::stream_base::client,
                        bind(
                            &Session::on_handshake,
                            shared_from_this(),
                            placeholders::_1
                        )
                    );
                } else {
                    http::async_write(
                        boost::get<plain_socket>(_socket),
                        _req,
                        bind(
                            &Session::on_write,
                            shared_from_this(),
                            placeholders::_1,
                            placeholders::_2
                        )
                    );
                }
            }

            void on_handshake(error_code code) {
                if (code) return log("handshake", code);
                http::async_write(
                    boost::get<ssl_socket>(_socket),
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
                if (code) return log("write", code);
                if (_secured) {
                    http::async_read(
                        boost::get<ssl_socket>(_socket),
                        _buffer,
                        _res,
                        bind(
                            &Session::on_read,
                            shared_from_this(),
                            placeholders::_1,
                            placeholders::_2
                        )
                    );
                } else {
                    http::async_read(
                        boost::get<plain_socket>(_socket),
                        _buffer,
                        _res,
                        bind(
                            &Session::on_read,
                            shared_from_this(),
                            placeholders::_1,
                            placeholders::_2
                        )
                    );
                }
            }

            void on_read(error_code code, size_t bytes_transferred) {
                ignore_unused(bytes_transferred);
                if (code) return log("read", code);

                _fn(_res);

                if (_secured) {
                    boost::get<ssl_socket>(_socket).async_shutdown(
                        bind(
                            &Session::on_shutdown,
                            shared_from_this(),
                            placeholders::_1
                        )
                    );
                } else {
                    boost::get<plain_socket>(_socket).shutdown(
                        tcp::socket::shutdown_both,
                        code
                    );

                    if (code && code != boost::system::errc::not_connected) {
                        return log("shutdown", code);
                    }
                }
            }

            void on_shutdown(error_code code) {
                if (code == eof) code.assign(0, code.category());
                if (code) return log("shutdown", code);
            }
        };
    }
}

#endif //SYNCAIDE_SERVER_SESSION_H