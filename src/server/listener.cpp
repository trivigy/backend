#include "logging.h"
#include "server/listener.h"

server::Listener::Listener(
    io_context &ioc,
    context &ctx,
    tcp::endpoint endp,
    shared_ptr<Router> router
) : _ctx(ctx),
    _acceptor(ioc),
    _socket(ioc),
    _router(move(router)) {
    error_code code;

    _acceptor.open(endp.protocol(), code);
    if (code) {
        log("open", code);
        return;
    }

    _acceptor.set_option(socket_base::reuse_address(true));
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
    if (!_acceptor.is_open()) return;
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
    if (code) log("accept", code);
    make_shared<Handoff>(move(_socket), _ctx, _router)->run();
    _acceptor.async_accept(
        _socket,
        bind(
            &Listener::on_accept,
            shared_from_this(),
            std::placeholders::_1
        )
    );
}