#ifndef SYNCAIDE_SERVER_HTTP_H
#define SYNCAIDE_SERVER_HTTP_H

#include "server/miner.h"
#include "server/response.h"
#include "server/helper.h"
#include "server/router.h"
#include "server/ssl_stream.h"
#include "server/server.h"
#include "common/uri.h"

#include <boost/beast/core.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <functional>
#include <string>
#include <chrono>

using namespace std;
extern map<string, vector<unsigned char>> resources;

namespace server {
    namespace asio = boost::asio;
    namespace ssl = boost::asio::ssl;
    namespace http = boost::beast::http;
    namespace websocket = boost::beast::websocket;
    namespace fs = boost::filesystem;
    using nlohmann::json;
    using chrono::seconds;
    using chrono::system_clock;
    using chrono::duration_cast;
    using boost::system::error_code;
    using boost::beast::http::request;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using boost::beast::http::field;
    using boost::beast::http::status;
    using boost::beast::http::verb;
    using boost::beast::string_view;
    using boost::beast::flat_buffer;
    using boost::beast::string_param;
    using boost::asio::bind_executor;
    using boost::asio::steady_timer;
    using boost::asio::ssl::context;
    using boost::asio::io_context;
    using boost::asio::ip::tcp;
    using boost::asio::ip::address;
    using boost::asio::strand;
    using boost::asio::error::operation_aborted;
    using boost::uuids::random_generator;
    using boost::uuids::uuid;
    using boost::lexical_cast;
    using boost::variant;
    using boost::tribool;
    using boost::algorithm::to_lower_copy;
    using common::Uri;

    class Server;

    class Http : public enable_shared_from_this<Http> {
        using plain_socket = tcp::socket;
        using ssl_socket = ssl_stream<tcp::socket>;
        using request_type = request<string_body>;
        using response_type = response<string_body>;
        using Socket = variant<plain_socket, ssl_socket>;

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
                            auto &s = boost::get<ssl_socket>(_self._socket);
                            http::async_write(
                                s, _msg,
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
                            auto &s = boost::get<plain_socket>(_self._socket);
                            http::async_write(
                                s, _msg,
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
                if (_items.size() == 1) (*_items.front())();
            }

        private:
            Http &_self;
            vector<unique_ptr<work>> _items;
        };

    private:
        Server &_server;
        request_type _req;
        flat_buffer _buffer;
        steady_timer _timer;
        bool _eof = false;
        context &_ctx;
        Router &_router;
        queue _queue;

    protected:
        Socket _socket;
        strand<io_context::executor_type> _strand;
        tribool _secured;

    public:
        Http(
            Server &server,
            context &ctx,
            Router &router,
            tcp::socket socket,
            flat_buffer buffer,
            tribool secured
        );

        void queue(response_type &&resp);

        Server &server();

        Socket &socket();

        context &ctx();

        steady_timer &timer();

        tribool &secured();

        void run();

    protected:
        void read();

        void eof();

        void timeout();

        void on_handshake(error_code code, size_t bytes_used);

        void on_timer(error_code code);

        void on_read(error_code code);

        void on_write(error_code code, bool close);

        void on_shutdown(error_code code);

    private:
        static Socket deduce_socket(
            tcp::socket socket,
            context &ctx,
            tribool secured
        );

    public:
        static int health(void *server, void *request);

        static int syncaide_js(void *server, void *request);

        static int syncaide_wasm(void *server, void *request);

        static int agent_uid(void *server, void *request, const string &uid);

#ifndef NDEBUG

        static int syncaide_html(void *server, void *request);

#endif //NDEBUG
    };
}

#endif //SYNCAIDE_SERVER_HTTP_H
