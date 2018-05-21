#include "logging.h"
#include "server/peering.h"

server::Peering::Peering(Server &server) :
    _server(server),
    _view(make_shared<View>()),
    _timer(_ioc, steady_time_point::max()) {
    auto cfg = _server.cfg();

    deque<Peer> buffer;
    for (auto &join : cfg.network.joins) {
        buffer.emplace_back(Peer(join.netloc(), 0));
    }
    _view->update(cfg.members.c, cfg.members.H, cfg.members.S, buffer);
}

void server::Peering::start() {
    grpc::ServerBuilder builder;
    rpc::services::MembersService service(_server);

    builder.AddListeningPort(
        _server.cfg().network.bind.netloc(),
        grpc::InsecureServerCredentials()
    );

    builder.RegisterService((Members::Service *) (&service));
    _rpc = builder.BuildAndStart();

    on_pulse({});

    _handlers.emplace_back([&] { _rpc->Wait(); });
    _handlers.emplace_back([&] { _ioc.run(); });
    server::exit.peering.get_future().wait();
    _rpc->Shutdown();
    _ioc.stop();
    for_each(_handlers.begin(), _handlers.end(), [](thread &t) { t.join(); });
}

shared_ptr<View> server::Peering::view() {
    return _view;
}

void server::Peering::on_pulse(error_code code) {
    auto cfg = _server.cfg();
    if (_view->empty()) {
        _timer.expires_after(chrono::seconds(1));
        _timer.async_wait(bind(&Peering::on_pulse, shared_from_this(), _1));
        return;
    }

    deque<Peer> push = _view->select(cfg.members.c / 2 - 1, cfg.members.H);
    push.emplace(push.begin(), Peer(cfg.network.advertise.netloc(), 0));

    auto peer = _view->random_peer();
    rpc::callers::MembersCaller caller(
        grpc::CreateCustomChannel(
            peer.addr(),
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    auto[status, pull] = caller.gossip(push, cfg.network.advertise.netloc());
    _view->update(cfg.members.c, cfg.members.H, cfg.members.S, pull.value());

    if (!status.ok()) {
        json extra = {{"peer", peer.addr()}};
        LOG(error) << logging::add_value("Extra", extra.dump())
                   << status.error_message();
    }

    _timer.expires_after(chrono::seconds(1));
    _timer.async_wait(bind(&Peering::on_pulse, shared_from_this(), _1));
}