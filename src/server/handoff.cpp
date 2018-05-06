#include "server/handoff.h"

server::Handoff::Handoff(
    tcp::socket socket,
    context &ctx,
    shared_ptr<Router> router
) : _socket(move(socket)),
    _ctx(ctx),
    _strand(_socket.get_executor()),
    _router(move(router)) {}

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
        move(_socket),
        move(_buffer),
        secured,
        _ctx,
        _router
    )->run();
}
