#include "logging.h"
#include "server/listener.h"

boost::beast::string_view server::mime_type(string_view path) {
    using boost::beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == boost::beast::string_view::npos)
            return boost::beast::string_view{};
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm")) return "text/html";
    if (iequals(ext, ".html")) return "text/html";
    if (iequals(ext, ".php")) return "text/html";
    if (iequals(ext, ".css")) return "text/css";
    if (iequals(ext, ".txt")) return "text/plain";
    if (iequals(ext, ".js")) return "application/javascript";
    if (iequals(ext, ".json")) return "application/json";
    if (iequals(ext, ".xml")) return "application/xml";
    if (iequals(ext, ".swf")) return "application/x-shockwave-flash";
    if (iequals(ext, ".flv")) return "video/x-flv";
    if (iequals(ext, ".png")) return "image/png";
    if (iequals(ext, ".jpe")) return "image/jpeg";
    if (iequals(ext, ".jpeg")) return "image/jpeg";
    if (iequals(ext, ".jpg")) return "image/jpeg";
    if (iequals(ext, ".gif")) return "image/gif";
    if (iequals(ext, ".bmp")) return "image/bmp";
    if (iequals(ext, ".ico")) return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff")) return "image/tiff";
    if (iequals(ext, ".tif")) return "image/tiff";
    if (iequals(ext, ".svg")) return "image/svg+xml";
    if (iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

std::string server::path_cat(string_view base, string_view path) {
    if (base.empty()) {
        return path.to_string();
    }
    std::string result = base.to_string();
    char constexpr path_separator = '/';
    if (result.back() == path_separator) {
        result.resize(result.size() - 1);
    }
    result.append(path.data(), path.size());
    return result;
}

void server::fail(error_code code, char const *what) {
    std::cerr << what << ": " << code.message() << std::endl;
}

// Websocket
// ########################################################################

server::Websocket::Websocket(
    tcp::socket socket,
    tribool secured,
    context &ctx
) : _strand(socket.get_executor().context().get_executor()),
    _timer(
        socket.get_executor().context(),
        chrono::time_point<chrono::steady_clock>::max()
    ),
    _socket(move(socket)),
    _plain(_socket),
    _secure(_socket, ctx),
    _secured(secured),
    _ctx(ctx) {}

void server::Websocket::run(http::request<http::string_body> &&req) {
    on_timer({});
    accept(move(req));
}

void server::Websocket::accept(http::request<http::string_body> &&req) {
    _timer.expires_after(chrono::seconds(15));
    if (_secured) {
        _secure.async_accept(
            req,
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_accept,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    } else {
        _plain.async_accept(
            req,
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_accept,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    }
}

void server::Websocket::read() {
    _timer.expires_after(chrono::seconds(15));
    if (_secured) {
        _secure.async_read(
            _buffer,
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_read,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    } else {
        _plain.async_read(
            _buffer,
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_read,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    }
}

void server::Websocket::timeout() {
    if (_secured) {
        if (_eof) {
            return;
        }

        _timer.expires_at(
            chrono::time_point<chrono::steady_clock>::max()
        );

        on_timer({});
        _eof = true;
        _timer.expires_after(chrono::seconds(15));
        _secure.next_layer().async_shutdown(
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_conclude,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    } else {
        if (_close) {
            return;
        }

        _close = true;
        _timer.expires_after(chrono::seconds(15));
        _plain.async_close(
            (const uint16_t) websocket::close_code::normal,
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_conclude,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    }
}

void server::Websocket::on_timer(error_code code) {
    if (code && code != operation_aborted) {
        cerr << "--- 11 ---" << endl;
        return fail(code, "timer");
    }

    if (_timer.expiry() <= chrono::steady_clock::now()) {
        cerr << "--- 16 ---" << endl;
        timeout();
    }

    _timer.async_wait(
        bind_executor(
            _strand,
            bind(
                &Websocket::on_timer,
                shared_from_this(),
                placeholders::_1
            )
        )
    );
}

void server::Websocket::on_accept(error_code code) {
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        cerr << "--- 10 ---" << endl;
        return fail(code, "accept");
    }

    cerr << "--- 18 ---" << endl;

    read();
}

void server::Websocket::on_conclude(error_code code) {
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        cerr << "--- 9 ---" << endl;
        return fail(code, "shutdown");
    }
}

void server::Websocket::on_read(error_code code, size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (code == operation_aborted) {
        return;
    }

    if (code == websocket::error::closed) {
        return;
    }

    if (code) {
        cerr << "--- 7 ---" << endl;
        fail(code, "read");
    }

    if (_secured) {
        _secure.text(_secure.got_text());
        _secure.async_write(
            _buffer.data(),
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_write,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    } else {
        _plain.text(_plain.got_text());
        _plain.async_write(
            _buffer.data(),
            bind_executor(
                _strand,
                bind(
                    &Websocket::on_write,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    }
}

void server::Websocket::on_write(error_code code, size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        cerr << "--- 6 ---" << endl;
        return fail(code, "write");
    }

    _buffer.consume(_buffer.size());
    read();
}

// Http
// ########################################################################

server::Http::Http(
    tcp::socket socket,
    flat_buffer buffer,
    tribool secured,
    context &ctx,
    string &root
) : _strand(socket.get_executor().context().get_executor()),
    _timer(
        socket.get_executor().context(),
        chrono::time_point<chrono::steady_clock>::max()
    ),
    _socket(move(socket)),
    _stream(_socket, ctx),
    _buffer(move(buffer)),
    _secured(secured),
    _queue(*this),
    _ctx(ctx),
    _root(root) {}

void server::Http::run() {
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
        read();
    }
}

void server::Http::read() {
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

void server::Http::eof() {
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
        _socket.shutdown(tcp::socket::shutdown_both, code);
        _socket.close(code);
    }
}

void server::Http::on_handshake(error_code code, size_t bytes_used) {
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        return fail(code, "handshake");
    }

    _buffer.consume(bytes_used);
    read();
}

void server::Http::on_timer(error_code code) {
    if (code && code != operation_aborted) {
        return fail(code, "timer");
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
    tcp::endpoint remote = _socket.remote_endpoint();
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
        }
    };
    LOG(info) << logging::add_value("Extra", extra.dump());

    if (code == operation_aborted) {
        return;
    }

    if (code == http::error::end_of_stream) {
        return eof();
    }

    if (code) {
        return fail(code, "read");
    }

    if (websocket::is_upgrade(_req)) {
        make_shared<Websocket>(
            move(_socket),
            _secured,
            _ctx
        )->run(move(_req));
        _timer.expires_at(chrono::time_point<chrono::steady_clock>::max());
        return;
    }

    handle_request(_root, move(_req), _queue);

    if (!_queue.is_full()) {
        read();
    }
}

void server::Http::on_write(error_code code, bool close) {
    if (code == operation_aborted) {
        return;
    }

    if (code) {
        return fail(code, "write");
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
        return fail(code, "shutdown");
    }
}

void server::Http::handle_request(
    string_view root,
    http::request<http::string_body> &&req,
    queue &send
) {
    auto const bad_request =
        [&req](string_view why) {
            http::response<http::string_body> res(
                http::status::bad_request,
                req.version()
            );
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
            http::response<http::string_body> res(
                http::status::not_found,
                req.version()
            );
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
            http::response<http::string_body> res(
                http::status::internal_server_error,
                req.version()
            );
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

    std::string path = path_cat(root, req.target());
    if (req.target().back() == '/') {
        path.append("index.html");
    }

    boost::beast::error_code code;
    http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::scan, code);

    if (code == boost::system::errc::no_such_file_or_directory) {
        return send(not_found(req.target()));
    }

    if (code) {
        return send(server_error(code.message()));
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

// Detector
// ########################################################################

server::Detector::Detector(tcp::socket socket, context &ctx, string &root) :
    _socket(move(socket)),
    _ctx(ctx),
    _strand(_socket.get_executor()),
    _root(root) {}

void server::Detector::run() {
    async_detect_ssl(
        _socket,
        _buffer,
        asio::bind_executor(
            _strand,
            bind(
                &Detector::on_detect,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        )
    );
}

void server::Detector::on_detect(error_code code, tribool secured) {
    if (code) {
        return fail(code, "detect");
    }

    make_shared<Http>(
        move(_socket),
        move(_buffer),
        secured,
        _ctx,
        _root
    )->run();
}

// Listener
// ########################################################################

server::Listener::Listener(
    io_context &ioc,
    context &ctx,
    tcp::endpoint endp,
    string &root
) : _ctx(ctx), _acceptor(ioc), _socket(ioc), _root(root) {
    error_code code;

    _acceptor.open(endp.protocol(), code);
    if (code) {
        fail(code, "open");
        return;
    }

    _acceptor.bind(endp, code);
    if (code) {
        fail(code, "bind");
        return;
    }

    // Start listening for connections
    _acceptor.listen(socket_base::max_listen_connections, code);
    if (code) {
        fail(code, "listen");
        return;
    }
}

void server::Listener::run() {
    if (!_acceptor.is_open()) {
        return;
    }
    accept();
}

void server::Listener::accept() {
    _acceptor.async_accept(
        _socket,
        bind(
            &Listener::on_accept,
            shared_from_this(),
            std::placeholders::_1
        )
    );
}

void server::Listener::on_accept(error_code code) {
    if (code) {
        fail(code, "accept");
    } else {
        make_shared<Detector>(move(_socket), _ctx, _root)->run();
    }
    accept();
}