#include "logging.h"
#include "server/listener.h"

server::Listener::Listener(
    io_context &ioc,
    context &ctx,
    tcp::endpoint endp,
    string &root
) : _ctx(ctx),
    _acceptor(ioc),
    _socket(ioc),
    _root(root) {
    error_code code;

    _acceptor.open(endp.protocol(), code);
    if (code) {
        log("open", code);
        return;
    }

    _acceptor.bind(endp, code);
    if (code) {
        log("bind", code);
        return;
    }

    _acceptor.listen(socket_base::max_listen_connections, code);
    if (code) {
        log("listen", code);
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
        log("accept", code);
    } else {
        make_shared<Detector>(move(_socket), _ctx, _root)->run();
    }
    accept();
}