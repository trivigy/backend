#include "logging.h"
#include "server/http.h"

server::Http::Http(
    tcp::socket socket,
    flat_buffer buffer,
    tribool secured,
    context &ctx,
    Router &router
) : _strand(socket.get_executor().context().get_executor()),
    _timer(
        socket.get_executor().context(),
        chrono::time_point<chrono::steady_clock>::max()
    ),
    _socket(deduce_socket(move(socket), ctx, secured)),
    _buffer(move(buffer)),
    _secured(secured),
    _queue(*this),
    _ctx(ctx),
    _router(router) {}

void server::Http::run() {
    on_timer({});
    if (_secured) {
        _timer.expires_after(chrono::seconds(15));
        get<ssl_stream<tcp::socket>>(_socket).async_handshake(
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
            get<ssl_stream<tcp::socket>>(_socket),
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
            get<tcp::socket>(_socket),
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
        get<ssl_stream<tcp::socket>>(_socket).async_shutdown(
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
        get<tcp::socket>(_socket).shutdown(tcp::socket::shutdown_send, code);
    }
}

void server::Http::timeout() {
    if (_secured) {
        if (_eof) {
            return;
        }

        _timer.expires_at(
            chrono::time_point<chrono::steady_clock>::max()
        );
        on_timer({});
        eof();
    } else {
        error_code code;
        get<tcp::socket>(_socket).shutdown(tcp::socket::shutdown_both, code);
        get<tcp::socket>(_socket).close(code);
    }
}

void server::Http::on_handshake(error_code code, size_t bytes_used) {
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        return log("handshake", code);
    }

    _buffer.consume(bytes_used);
    read();
}

void server::Http::on_timer(error_code code) {
    if (code && code != operation_aborted) {
        return log("timer", code);
    }

    if (_timer.expiry() <= chrono::steady_clock::now()) {
        return timeout();
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

void server::Http::on_read(error_code code) {
    if (code == operation_aborted) {
        return;
    }

    if (code == http::error::end_of_stream) {
        return eof();
    }

    if (code) {
        return log("read", code);
    }

    auto fields = json::object();
    for (auto &field : _req) {
        fields.emplace(string(field.name_string()), string(field.value()));
    }

    tcp::endpoint remote;
    if (_secured) {
        remote = get<ssl_stream<tcp::socket>>(_socket)
            .next_layer()
            .remote_endpoint();
    } else {
        remote = get<tcp::socket>(_socket).remote_endpoint();
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

    response_type resp = _router.dispatch(_req);
    if (resp.result() != status::switching_protocols) {
        _queue(move(resp));
    } else {
        auto params = json::parse(resp.body());
        if (websocket::is_upgrade(_req)) {
            if (_secured) {
                make_shared<Websocket>(
                    move(get<ssl_stream<tcp::socket>>(_socket)),
                    _secured,
                    _ctx,
                    move(params)
                )->run(move(_req));
            } else {
                make_shared<Websocket>(
                    move(get<tcp::socket>(_socket)),
                    _secured,
                    _ctx,
                    move(params)
                )->run(move(_req));
            }
            _timer.expires_at(chrono::time_point<chrono::steady_clock>::max());
            return;
        } else {
            _queue(Response::bad_request(_req));
        }
    }

    if (!_queue.is_full()) {
        read();
    }
}

void server::Http::on_write(error_code code, bool close) {
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        return log("write", code);
    }

    if (close) {
        return eof();
    }

    if (_queue.on_write()) {
        read();
    }
}

void server::Http::on_shutdown(error_code code) {
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        return log("shutdown", code);
    }
}

server::Http::response_type
server::Http::syncaide_js(request_type &req) {
    if (req.method() != verb::head && req.method() != verb::get) {
        return Response::method_not_allowed(req);
    }

    auto search = resources.find("syncaide.js");
    if (search != resources.end()) {
        response<string_body> resp(status::ok, req.version());
        resp.set(field::server, string_param(BOOST_BEAST_VERSION_STRING));
        resp.set(field::content_type, string_param("application/javascript"));
        if (req.method() == verb::head) {
            resp.content_length(search->second.size());
            resp.keep_alive(req.keep_alive());
            return resp;
        }

        resp.content_length(search->second.size());
        resp.keep_alive(req.keep_alive());
        resp.body() = string(search->second.begin(), search->second.end());
        resp.prepare_payload();
        return resp;
    }

    return Response::internal_server_error(req);
}

server::Http::response_type
server::Http::syncaide_wasm(request_type &req) {
    if (req.method() != verb::head && req.method() != verb::get) {
        return Response::method_not_allowed(req);
    }

    auto search = resources.find("syncaide.wasm");
    if (search != resources.end()) {
        response<string_body> resp(status::ok, req.version());
        resp.set(field::server, string_param(BOOST_BEAST_VERSION_STRING));
        resp.set(field::content_type, string_param("application/wasm"));
        if (req.method() == verb::head) {
            resp.content_length(search->second.size());
            resp.keep_alive(req.keep_alive());
            return resp;
        }

        resp.content_length(search->second.size());
        resp.keep_alive(req.keep_alive());
        resp.body() = string(search->second.begin(), search->second.end());
        resp.prepare_payload();
        return resp;
    }

    return Response::internal_server_error(req);
}

server::Http::response_type
server::Http::agent_uid(request_type &req, const string &uid) {
    json params = {{"uid", uid}};
    response<string_body> resp(status::switching_protocols, req.version());
    resp.body() = params.dump();
    return resp;
}

server::Http::Socket server::Http::deduce_socket(
    tcp::socket socket,
    context &ctx,
    tribool secured
) {
    if (secured) {
        return ssl_stream<tcp::socket>(move(socket), ctx);
    } else {
        return tcp::socket(move(socket));
    }
}
