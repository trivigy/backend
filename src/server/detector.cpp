#include "server/detector.h"

server::Detector::Detector(
    tcp::socket socket,
    context &ctx,
    Router &router
) :
    _socket(move(socket)),
    _ctx(ctx),
    _strand(_socket.get_executor()),
    _router(router) {}

void server::Detector::run() {
    async_detect_ssl(
        _socket,
        _buffer,
        bind_executor(
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
        log("detector/on_detect", code);
    }

    make_shared<Http>(
        move(_socket),
        move(_buffer),
        secured,
        _ctx,
        _router
    )->run();
}