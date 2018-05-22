#include "logging.h"
#include "server/server.h"

server::exit_t server::exit; // NOLINT

server::Server::Server(Options &options) :
    _cfg(options),
    _peering(make_shared<Peering>(*this)),
    _upstream(make_shared<Upstream>(*this)),
    _frontend(make_shared<Frontend>(*this)) {}

server::Options &server::Server::cfg() {
    return _cfg;
}

int server::Server::start() {
    _handlers.emplace_back(thread([=] { _peering->start(); }));
    _handlers.emplace_back(thread([=] { _upstream->start(); }));
    _handlers.emplace_back(thread([=] { _frontend->start(); }));

    for (auto &thread : _handlers) {
        thread.join();
    }

    return EXIT_SUCCESS;
}

shared_ptr<server::Peering> server::Server::peering() {
    return _peering;
}

shared_ptr<server::Upstream> server::Server::upstream() {
    return _upstream;
}

shared_ptr<server::Frontend> server::Server::frontend() {
    return _frontend;
}
