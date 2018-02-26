#ifndef SYNC_SERVER_LISTENER_H
#define SYNC_SERVER_LISTENER_H

#include "server/detector.h"
#include "server/stream.h"
#include "server/queue.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std;

namespace server {
    using tcp = boost::asio::ip::tcp;
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;
    namespace websocket = boost::beast::websocket;

    using boost::beast::string_view;

    string_view mime_type(string_view path);

    using boost::beast::string_view;

    string path_cat(string_view base, string_view path);

    using boost::beast::string_view;

    template<class Body, class Allocator, class Send>
    void handle_request(
        string_view doc_root,
        http::request<Body, http::basic_fields<Allocator>> &&req,
        Send &&send
    ) {
        auto const bad_request =
            [&req](string_view why) {
                http::response<http::string_body> res{http::status::bad_request, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = why.to_string();
                res.prepare_payload();
                return res;
            };

        // Returns a not found response
        auto const not_found =
            [&req](string_view target) {
                http::response<http::string_body> res{http::status::not_found, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() =
                    "The resource '" + target.to_string() + "' was not found.";
                res.prepare_payload();
                return res;
            };

        // Returns a server error response
        auto const server_error =
            [&req](string_view what) {
                http::response<http::string_body> res{http::status::internal_server_error, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "An error occurred: '" + what.to_string() + "'";
                res.prepare_payload();
                return res;
            };

        // Make sure we can handle the method
        if (req.method() != http::verb::get &&
            req.method() != http::verb::head)
            return send(bad_request("Unknown HTTP-method"));

        // Request path must be absolute and not contain "..".
        if (req.target().empty() ||
            req.target()[0] != '/' ||
            req.target().find("..") != string_view::npos)
            return send(bad_request("Illegal request-target"));

        std::string path = path_cat(doc_root, req.target());
        if (req.target().back() == '/') {
            path.append("index.html");
        }

        boost::beast::error_code ec;
        http::file_body::value_type body;
        body.open(path.c_str(), boost::beast::file_mode::scan, ec);

        if (ec == boost::system::errc::no_such_file_or_directory) {
            return send(not_found(req.target()));
        }

        if (ec) {
            return send(server_error(ec.message()));
        }

        if (req.method() == http::verb::head) {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, mime_type(path));
            res.content_length(body.size());
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }

        http::response<http::file_body> res{std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(body.size());
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    void fail(boost::system::error_code code, char const *what);

// ########################################################################

//    class websocket_session : public enable_shared_from_this<websocket_session> {
//    public:
//        explicit websocket_session(
//            boost::asio::io_context &ioc
//        ) : strand_(ioc.get_executor()),
//            timer_(ioc, chrono::time_point<chrono::steady_clock>::max()) {}
//
//        template<class Body, class Allocator>
//        void do_accept(http::request<Body, http::basic_fields<Allocator>> req) {
//            timer_.expires_after(std::chrono::seconds(15));
//            derived().ws().async_accept(
//                req,
//                boost::asio::bind_executor(
//                    strand_,
//                    std::bind(
//                        &websocket_session::on_accept,
//                        derived().shared_from_this(),
//                        std::placeholders::_1
//                    )
//                )
//            );
//        }
//
//        void on_timer(boost::system::error_code ec) {
//            if (ec && ec != boost::asio::error::operation_aborted) {
//                return fail(ec, "timer");
//            }
//
//            if (timer_.expiry() <= std::chrono::steady_clock::now()) {
//                derived().do_timeout();
//            }
//
//            timer_.async_wait(
//                boost::asio::bind_executor(
//                    strand_,
//                    std::bind(
//                        &websocket_session::on_timer,
//                        derived().shared_from_this(),
//                        std::placeholders::_1
//                    )
//                )
//            );
//        }
//
//        void on_accept(boost::system::error_code ec) {
//            if (ec == boost::asio::error::operation_aborted) {
//                return;
//            }
//
//            if (ec) {
//                return fail(ec, "accept");
//            }
//
//            do_read();
//        }
//
//        void do_read() {
//            timer_.expires_after(std::chrono::seconds(15));
//            derived().ws().async_read(
//                buffer_,
//                boost::asio::bind_executor(
//                    strand_,
//                    std::bind(
//                        &websocket_session::on_read,
//                        derived().shared_from_this(),
//                        std::placeholders::_1,
//                        std::placeholders::_2
//                    )
//                )
//            );
//        }
//
//        void on_read(
//            boost::system::error_code ec,
//            std::size_t bytes_transferred
//        ) {
//            boost::ignore_unused(bytes_transferred);
//
//            if (ec == boost::asio::error::operation_aborted) {
//                return;
//            }
//
//
//            if (ec == websocket::error::closed) {
//                return;
//            }
//
//
//            if (ec) {
//                fail(ec, "read");
//            }
//
//
//            // Echo the message
//            derived().ws().text(derived().ws().got_text());
//            derived().ws().async_write(
//                buffer_.data(),
//                boost::asio::bind_executor(
//                    strand_,
//                    std::bind(
//                        &websocket_session::on_write,
//                        derived().shared_from_this(),
//                        std::placeholders::_1,
//                        std::placeholders::_2
//                    )
//                )
//            );
//        }
//
//        void on_write(
//            boost::system::error_code ec,
//            std::size_t bytes_transferred
//        ) {
//            boost::ignore_unused(bytes_transferred);
//            if (ec == boost::asio::error::operation_aborted) {
//                return;
//            }
//
//            if (ec) {
//                return fail(ec, "write");
//            }
//
//            buffer_.consume(buffer_.size());
//            do_read();
//        }
//
//    private:
//        boost::beast::multi_buffer buffer_;
//
//    protected:
//        boost::asio::strand<boost::asio::io_context::executor_type> strand_;
//        boost::asio::steady_timer timer_;
//    };

//    class plain_websocket_session :
//        public websocket_session<plain_websocket_session>,
//        public std::enable_shared_from_this<plain_websocket_session> {
//    public:
//        explicit plain_websocket_session(tcp::socket socket) :
//            websocket_session<plain_websocket_session>(socket.get_executor().context()),
//            ws_(std::move(socket)) {}
//
//        websocket::stream<tcp::socket> &ws() {
//            return ws_;
//        }
//
//        template<class Body, class Allocator>
//        void run(http::request<Body, http::basic_fields<Allocator>> req) {
//            on_timer({});
//            do_accept(std::move(req));
//        }
//
//        void do_timeout() {
//            if (close_) {
//                return;
//            }
//
//            close_ = true;
//            timer_.expires_after(std::chrono::seconds(15));
//            ws_.async_close(
//                websocket::close_code::normal,
//                boost::asio::bind_executor(
//                    strand_,
//                    std::bind(
//                        &plain_websocket_session::on_close,
//                        shared_from_this(),
//                        std::placeholders::_1
//                    )
//                )
//            );
//        }
//
//        void on_close(boost::system::error_code ec) {
//            if (ec == boost::asio::error::operation_aborted) {
//                return;
//            }
//
//            if (ec) {
//                return fail(ec, "close");
//            }
//        }
//
//    private:
//        websocket::stream<tcp::socket> ws_;
//        bool close_ = false;
//    };
//
//    class ssl_websocket_session :
//        public websocket_session<ssl_websocket_session>,
//        public std::enable_shared_from_this<ssl_websocket_session> {
//    public:
//        explicit ssl_websocket_session(Stream<tcp::socket> stream) :
//            websocket_session<ssl_websocket_session>(stream.get_executor().context()),
//            ws_(std::move(stream)),
//            strand_(ws_.get_executor()) {}
//
//        websocket::stream<Stream<tcp::socket>> &ws() {
//            return ws_;
//        }
//
//        template<class Body, class Allocator>
//        void run(http::request<Body, http::basic_fields<Allocator>> req) {
//            on_timer({});
//            do_accept(std::move(req));
//        }
//
//        void do_eof() {
//            eof_ = true;
//            timer_.expires_after(std::chrono::seconds(15));
//            ws_.next_layer().async_shutdown(
//                boost::asio::bind_executor(
//                    strand_,
//                    std::bind(
//                        &ssl_websocket_session::on_shutdown,
//                        shared_from_this(),
//                        std::placeholders::_1
//                    )
//                )
//            );
//        }
//
//        void on_shutdown(boost::system::error_code ec) {
//            if (ec == boost::asio::error::operation_aborted) {
//                return;
//            }
//
//            if (ec) {
//                return fail(ec, "shutdown");
//            }
//        }
//
//        void do_timeout() {
//            if (eof_) {
//                return;
//            }
//
//            timer_.expires_at(chrono::time_point<chrono::steady_clock>::max());
//            on_timer({});
//            do_eof();
//        }
//
//    private:
//        websocket::stream<Stream<tcp::socket>> ws_;
//        boost::asio::strand<boost::asio::io_context::executor_type> strand_;
//        bool eof_ = false;
//    };
//
//    template<class Body, class Allocator>
//    void make_websocket_session(
//        tcp::socket socket,
//        http::request<Body, http::basic_fields<Allocator>> req
//    ) {
//        std::make_shared<plain_websocket_session>(std::move(socket))->run(
//            std::move(req));
//    }
//
//    template<class Body, class Allocator>
//    void make_websocket_session(
//        Stream<tcp::socket> stream,
//        http::request<Body, http::basic_fields<Allocator>> req
//    ) {
//        std::make_shared<ssl_websocket_session>(std::move(stream))->run(std::move(req));
//    }

// ########################################################################

    namespace asio = boost::asio;
    namespace http = boost::beast::http;
    using boost::system::error_code;
    using boost::beast::flat_buffer;
    using boost::asio::bind_executor;
    using boost::asio::steady_timer;
    using boost::asio::ssl::context;
    using boost::asio::io_context;
    using boost::asio::strand;
    using boost::asio::error::operation_aborted;
    using boost::tribool;

    class Http : public enable_shared_from_this<Http> {
        class queue {
            enum {
                limit = 8
            };

        public:
            struct work {
                virtual ~work() = default;

                virtual void operator()() = 0;
            };

            explicit queue(Http &self) : _self(self) {
                static_assert(limit > 0, "queue limit must be positive");
                _items.reserve(limit);
            }

            bool is_full() const {
                return _items.size() >= limit;
            }

            bool on_write() {
                BOOST_ASSERT(!_items.empty());
                auto const was_full = is_full();
                _items.erase(_items.begin());
                if (!_items.empty()) {
                    (*_items.front())();
                }
                return was_full;
            }

            template<bool isRequest, class Body, class Fields>
            void operator()(http::message<isRequest, Body, Fields> &&msg) {
                struct work_impl : work {
                    Http &_self;
                    http::message<isRequest, Body, Fields> _msg;

                    work_impl(
                        Http &self,
                        http::message<isRequest, Body, Fields> &&msg
                    ) : _self(self),
                        _msg(move(msg)) {}

                    void operator()() override {
                        if (_self._secured) {
                            http::async_write(
                                _self._stream,
                                _msg,
                                bind_executor(
                                    _self._strand,
                                    bind(
                                        &Http::on_write,
                                        _self.shared_from_this(),
                                        placeholders::_1,
                                        (bool) _msg.need_eof()
                                    )
                                )
                            );
                        } else {
                            http::async_write(
                                _self._socket,
                                _msg,
                                bind_executor(
                                    _self._strand,
                                    bind(
                                        &Http::on_write,
                                        _self.shared_from_this(),
                                        placeholders::_1,
                                        (bool) _msg.need_eof()
                                    )
                                )
                            );
                        }
                    }
                };

                _items.emplace_back(new work_impl(_self, move(msg)));
                if (_items.size() == 1) {
                    (*_items.front())();
                }
            }

        private:
            Http &_self;
            vector<unique_ptr<work>> _items;
        };

    public:
        Http(
            tcp::socket socket,
            flat_buffer buffer,
            tribool secured,
            context &ctx,
            string &root
        ) : _root(root),
            _queue(*this),
            _socket(move(socket)),
            _stream(_socket, ctx),
            _secured(secured),
            _timer(
                socket.get_executor().context(),
                chrono::time_point<chrono::steady_clock>::max()
            ),
            _strand(socket.get_executor().context().get_executor()),
            _buffer(move(buffer)) {}

        void run() {
            on_timer({});
            if (_secured) {
                _timer.expires_after(chrono::seconds(15));
                _stream.async_handshake(
                    ssl::stream_base::server,
                    _buffer.data(),
                    bind_executor(
                        _strand,
                        bind(
                            &Http::on_handshake,
                            shared_from_this(),
                            placeholders::_1,
                            placeholders::_2
                        )
                    )
                );
            } else {
                do_read();
            }
        }

        void on_handshake(error_code code, size_t bytes_used) {
            if (code == operation_aborted) {
                return;
            }

            if (code) {
                return fail(code, "handshake");
            }

            _buffer.consume(bytes_used);
            do_read();
        }

        void do_read() {
            _timer.expires_after(chrono::seconds(15));
            if (_secured) {
                http::async_read(
                    _stream,
                    _buffer,
                    _req,
                    bind_executor(
                        _strand,
                        bind(
                            &Http::on_read,
                            shared_from_this(),
                            placeholders::_1
                        )
                    )
                );
            } else {
                http::async_read(
                    _socket,
                    _buffer,
                    _req,
                    bind_executor(
                        _strand,
                        bind(
                            &Http::on_read,
                            shared_from_this(),
                            placeholders::_1
                        )
                    )
                );
            }
        }

        void on_timer(error_code code) {
            if (code && code != operation_aborted) {
                return fail(code, "timer");
            }

            if (_timer.expiry() <= chrono::steady_clock::now()) {
                return do_timeout();
            }

            _timer.async_wait(
                bind_executor(
                    _strand,
                    bind(
                        &Http::on_timer,
                        shared_from_this(),
                        placeholders::_1
                    )
                )
            );
        }

        void on_read(error_code code) {
            if (code == operation_aborted) {
                return;
            }

            if (code == http::error::end_of_stream) {
                return do_eof();
            }

            if (code) {
                return fail(code, "read");
            }

            if (websocket::is_upgrade(_req)) {
                return fail(code, "websocket::is_upgrade(_req)");
//                return make_websocket_session(derived().release_stream(), move(_req));
            }

            handle_request(_root, move(_req), _queue);

            if (!_queue.is_full()) {
                do_read();
            }
        }

        void on_write(error_code code, bool close) {
            if (code == operation_aborted) {
                return;
            }

            if (code) {
                return fail(code, "write");
            }

            if (close) {
                return do_eof();
            }

            if (_queue.on_write()) {
                do_read();
            }
        }

        void do_eof() {
            if (_secured) {
                _eof = true;
                _timer.expires_after(chrono::seconds(15));
                _stream.async_shutdown(
                    bind_executor(
                        _strand,
                        bind(
                            &Http::on_shutdown,
                            shared_from_this(),
                            placeholders::_1
                        )
                    )
                );
            } else {
                error_code code;
                _socket.shutdown(tcp::socket::shutdown_send, code);
            }
        }

        void on_shutdown(error_code code) {
            if (code == operation_aborted) {
                return;
            }

            if (code) {
                return fail(code, "shutdown");
            }
        }

        void do_timeout() {
            if (_secured) {
                if (_eof) {
                    return;
                }

                _timer.expires_at(
                    chrono::time_point<chrono::steady_clock>::max()
                );
                on_timer({});
                do_eof();
            } else {
                error_code code;
                _socket.shutdown(tcp::socket::shutdown_both, code);
                _socket.close(code);
            }
        }

    private:
        http::request<http::string_body> _req;
        flat_buffer _buffer;
        steady_timer _timer;
        bool _eof = false;
        string &_root;
        queue _queue;

    protected:
        tcp::socket _socket;
        ssl::stream<tcp::socket&> _stream;
        strand<io_context::executor_type> _strand;
        tribool _secured;
    };

// Detector
// ########################################################################

    namespace asio = boost::asio;
    namespace beast = boost::beast;
    using boost::tribool;
    using boost::system::error_code;
    using boost::asio::ip::tcp;
    using boost::asio::ssl::context;
    using boost::asio::strand;

    class Detector : public enable_shared_from_this<Detector> {
    public:
        explicit Detector(tcp::socket socket, context &ctx, string &root);

        void run();

        void on_detect(error_code code, tribool secured);

    private:
        tcp::socket _socket;
        context &_ctx;
        strand<asio::io_context::executor_type> _strand;
        string &_root;
        beast::flat_buffer _buffer;
    };

// Listener
// ########################################################################

    using boost::system::error_code;
    using boost::asio::socket_base;
    using boost::asio::io_context;
    using boost::asio::ssl::context;

    class Listener : public std::enable_shared_from_this<Listener> {
    public:
        Listener(
            io_context &ioc,
            context &ctx,
            tcp::endpoint endp,
            string &root
        );

        void run();

        void do_accept();

        void on_accept(error_code code);

    private:
        context &_ctx;
        tcp::acceptor _acceptor;
        tcp::socket _socket;
        string &_root;
    };
}

#endif //SYNC_SERVER_LISTENER_H
