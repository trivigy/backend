#include "server/web/session.h"

void server::web::Session::run(Uri &uri) {
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

void server::web::Session::on_resolve(
    error_code code,
    tcp::resolver::results_type results
) {
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

void server::web::Session::on_connect(error_code code) {
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

void server::web::Session::on_handshake(error_code code) {
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

void server::web::Session::on_write(error_code code, size_t bytes_transferred) {
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

void server::web::Session::on_read(error_code code, size_t bytes_transferred) {
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

void server::web::Session::on_shutdown(error_code code) {
    if (code == eof) code.assign(0, code.category());
    if (code) return log("shutdown", code);
}