#include "logging.h"
#include "server/websocket.h"

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
        return log("timer", code);
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
        return log("accept", code);
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
        return log("shutdown", code);
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
        log("read", code);
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
        return log("write", code);
    }

    _buffer.consume(_buffer.size());
    read();
}