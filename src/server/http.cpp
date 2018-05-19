#include "logging.h"
#include "server/http.h"

server::Http::Http(
    Server &server,
    context &ctx,
    Router &router,
    tcp::socket socket,
    flat_buffer buffer,
    tribool secured
) : _server(server),
    _ctx(ctx),
    _router(router),
    _strand(socket.get_executor().context().get_executor()),
    _timer(socket.get_executor().context(), steady_time_point::max()),
    _socket(deduce_socket(move(socket), ctx, secured)),
    _buffer(move(buffer)),
    _secured(secured),
    _queue(*this) {}

void server::Http::queue(response_type &&resp) {
    _queue(move(resp));
}

server::Server &server::Http::server() {
    return _server;
}

server::Http::Socket &server::Http::socket() {
    return _socket;
}

boost::asio::ssl::context &server::Http::ctx() {
    return _ctx;
}

boost::asio::steady_timer &server::Http::timer() {
    return _timer;
}

boost::tribool &server::Http::secured() {
    return _secured;
}

void server::Http::run() {
    on_timer({});
    if (_secured) {
        _timer.expires_after(chrono::seconds(15));
        boost::get<ssl_socket>(_socket).async_handshake(
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
        read();
    }
}

void server::Http::read() {
    _timer.expires_after(chrono::seconds(15));
    if (_secured) {
        http::async_read(
            boost::get<ssl_socket>(_socket),
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
            boost::get<plain_socket>(_socket),
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

void server::Http::eof() {
    if (_secured) {
        _eof = true;
        _timer.expires_after(chrono::seconds(15));
        boost::get<ssl_socket>(_socket).async_shutdown(
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
        auto &socket = boost::get<plain_socket>(_socket);
        socket.shutdown(tcp::socket::shutdown_send, code);
    }
}

void server::Http::timeout() {
    if (_secured) {
        if (_eof) return;
        _timer.expires_at(steady_time_point::max());
        on_timer({});
        eof();
    } else {
        error_code code;
        auto &socket = boost::get<plain_socket>(_socket);
        socket.shutdown(tcp::socket::shutdown_both, code);
        socket.close(code);
    }
}

void server::Http::on_handshake(error_code code, size_t bytes_used) {
    if (code == operation_aborted) return;
    if (code) return log("handshake", code);
    _buffer.consume(bytes_used);
    read();
}

void server::Http::on_timer(error_code code) {
    if (code && code != operation_aborted) return log("timer", code);
    if (_timer.expiry() <= chrono::steady_clock::now()) return timeout();
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

void server::Http::on_read(error_code code) {
    if (code == operation_aborted) return;
    if (code == http::error::end_of_stream) return eof();
    if (code) return log("read", code);

    auto fields = json::object();
    for (auto &field : _req) {
        fields.emplace(string(field.name_string()), string(field.value()));
    }

    tcp::endpoint remote;
    if (_secured) {
        remote = boost::get<ssl_socket>(_socket).next_layer().remote_endpoint();
    } else {
        remote = boost::get<plain_socket>(_socket).remote_endpoint();
    }

    json extra = {
        {"remote",
            {
                {"addr", remote.address().to_string()},
                {"port", remote.port()},
            }
        },
        {"method", string(
            _req.method_string().data(),
            _req.method_string().size()
        )},
        {"target", string(_req.target().data(), _req.target().size())},
        {"version",
            to_string(_req.version() / 10)
            + "."
            + to_string(_req.version() % 10)
        },
        {"fields", fields}
    };
    LOG(info) << logging::add_value("Extra", extra.dump());

    auto resp = _router.dispatch(*this, _req);
    if (resp == (int) status::switching_protocols) return;
    if (!_queue.is_full()) read();
}

void server::Http::on_write(error_code code, bool close) {
    if (code == operation_aborted) return;
    if (code) return log("write", code);
    if (close) return eof();
    if (_queue.on_write()) read();
}

void server::Http::on_shutdown(error_code code) {
    if (code == operation_aborted) return;
    if (code) return log("shutdown", code);
}

server::Http::Socket server::Http::deduce_socket(
    tcp::socket socket,
    context &ctx,
    tribool secured
) {
    if (secured) {
        return ssl_socket(move(socket), ctx);
    } else {
        return plain_socket(move(socket));
    }
}

int server::Http::health(void *server, void *request) {
    auto srv = (Http *) server;
    auto req = (request_type *) request;
    response<string_body> resp(status::ok, req->version());
    resp.set(field::server, string_param(BOOST_BEAST_VERSION_STRING));
    srv->queue(move(resp));
    return (int) status::ok;
}

int server::Http::syncaide_js(void *server, void *request) {
    auto srv = (Http *) server;
    auto req = (request_type *) request;
    if (req->method() != verb::head && req->method() != verb::get) {
        srv->queue(Response::method_not_allowed(*req));
        return (int) status::method_not_allowed;
    }

    auto search = resources.find("syncaide.js");
    if (search != resources.end()) {
        response<string_body> resp(status::ok, req->version());
        resp.set(field::server, string_param(BOOST_BEAST_VERSION_STRING));
        resp.set(field::content_type, string_param("application/javascript"));
        if (req->method() == verb::head) {
            resp.content_length(search->second.size());
            resp.keep_alive(req->keep_alive());
            srv->queue(move(resp));
            return (int) status::ok;
        }

        string body(search->second.begin(), search->second.end());
        CryptoPP::SHA256 sha256sum;

        string sha256;
        CryptoPP::StringSource(
            body, true,
            new CryptoPP::HashFilter(
                sha256sum,
                new CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(sha256)
                )
            )
        );
        to_lower(sha256);

        uuid uid = random_generator{}();
        string path = fmt::format("/agent/{}", to_string(uid));
        long epoch = duration_cast<seconds>(
            system_clock::now().time_since_epoch()
        ).count();

        string params(json{
            {"id", to_string(uid)},
            {"addr", Uri("ws", "127.0.0.1", 8080, path).compose()},
            {"sha256", sha256},
            {"epoch", epoch}
        }.dump());

        string digest;
        CryptoPP::StringSource(
            params, true,
            new CryptoPP::HashFilter(
                sha256sum,
                new CryptoPP::HexEncoder(
                    new CryptoPP::StringSink(digest)
                )
            )
        );
        to_lower(digest);

        // TODO generate ecdsa signature from the digest

        json prepend = {{"arguments", {"signature", digest, params}}};
        body.insert(0, fmt::format("var Module = {0};\n", prepend.dump()));

        resp.content_length(body.size());
        resp.keep_alive(req->keep_alive());
        resp.body() = body;
        resp.prepare_payload();
        srv->queue(move(resp));
        return (int) status::ok;
    }

    srv->queue(Response::internal_server_error(*req));
    return (int) status::internal_server_error;
}

int server::Http::syncaide_wasm(void *server, void *request) {
    auto srv = (Http *) server;
    auto req = (request_type *) request;
    if (req->method() != verb::head && req->method() != verb::get) {
        srv->queue(Response::method_not_allowed(*req));
        return (int) status::method_not_allowed;
    }

    auto search = resources.find("syncaide.wasm");
    if (search != resources.end()) {
        response<string_body> resp(status::ok, req->version());
        resp.set(field::server, string_param(BOOST_BEAST_VERSION_STRING));
        resp.set(field::content_type, string_param("application/wasm"));
        if (req->method() == verb::head) {
            resp.content_length(search->second.size());
            resp.keep_alive(req->keep_alive());
            srv->queue(move(resp));
            return (int) status::ok;
        }

        resp.content_length(search->second.size());
        resp.keep_alive(req->keep_alive());
        resp.body() = string(search->second.begin(), search->second.end());
        resp.prepare_payload();
        srv->queue(move(resp));
        return (int) status::ok;
    }

    srv->queue(Response::internal_server_error(*req));
    return (int) status::internal_server_error;
}

int server::Http::agent_uid(void *server, void *request, const string &uid) {
    auto srv = (Http *) server;
    auto req = (request_type *) request;
    if (websocket::is_upgrade(*req)) {
        if (srv->secured()) {
            make_shared<Miner>(
                srv->server(),
                srv->ctx(),
                move(boost::get<ssl_socket>(srv->socket())),
                srv->secured(),
                uid
            )->run(move(*req));
        } else {
            make_shared<Miner>(
                srv->server(),
                srv->ctx(),
                move(boost::get<plain_socket>(srv->socket())),
                srv->secured(),
                uid
            )->run(move(*req));
        }
        srv->timer().expires_at(steady_time_point::max());
        return (int) status::switching_protocols;
    }

    srv->queue(Response::bad_request(*req));
    return (int) status::bad_request;
}

#ifndef NDEBUG

int server::Http::syncaide_html(void *server, void *request) {
    auto srv = (Http *) server;
    auto req = (request_type *) request;
    if (req->method() != verb::head && req->method() != verb::get) {
        srv->queue(Response::method_not_allowed(*req));
        return (int) status::method_not_allowed;
    }

    auto search = resources.find("syncaide.html");
    if (search != resources.end()) {
        response<string_body> resp(status::ok, req->version());
        resp.set(field::server, string_param(BOOST_BEAST_VERSION_STRING));
        resp.set(field::content_type, string_param("text/html"));
        if (req->method() == verb::head) {
            resp.content_length(search->second.size());
            resp.keep_alive(req->keep_alive());
            srv->queue(move(resp));
            return (int) status::ok;
        }

        resp.content_length(search->second.size());
        resp.keep_alive(req->keep_alive());
        resp.body() = string(search->second.begin(), search->second.end());
        resp.prepare_payload();
        srv->queue(move(resp));
        return (int) status::ok;
    }

    srv->queue(Response::internal_server_error(*req));
    return (int) status::internal_server_error;
}

#endif //NDEBUG
