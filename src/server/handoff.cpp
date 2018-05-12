#include "server/handoff.h"

server::Handoff::Handoff(
    Server &server,
    context &ctx,
    shared_ptr<Router> router,
    tcp::socket socket
) : _server(server),
    _ctx(ctx),
    _router(move(router)),
    _socket(move(socket)),
    _strand(_socket.get_executor()) {}

void server::Handoff::run() {
    async_detect_ssl(
        _socket,
        _buffer,
        bind_executor(
            _strand,
            bind(
                &Handoff::on_detect,
                shared_from_this(),
                placeholders::_1,
                placeholders::_2
            )
        )
    );
}

void server::Handoff::on_detect(error_code code, tribool secured) {
    if (code) log("detector/on_detect", code);
    make_shared<Http>(
        _server,
        _ctx,
        _router,
        move(_socket),
        move(_buffer),
        secured
    )->run();
}
