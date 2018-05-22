#include "logging.h"
#include "rpc/services/peers.h"

rpc::services::PeersService::PeersService(server::Server &server) :
    _server(server) {}

grpc::Status rpc::services::PeersService::gossip(
    grpc::ServerContext *context,
    const peers::GossipRequest *request,
    peers::GossipResponse *response
) {
    log("/peers/gossip", context->peer());

    deque<Peer> buffer;
    auto cfg = _server.cfg();
    auto peers = request->peers();
    auto it = peers.begin();
    while (it != peers.end()) {
        auto peer = *it;
        if (cfg.network.advertise.netloc().compare(0, peer.size(), peer)) {
            buffer.emplace_back(Peer(peer, 0));
        }
        it++;
    }

    auto view = _server.peering()->view();
    deque<Peer> result = view->select(cfg.peers.c / 2 - 1, cfg.peers.H);
    result.emplace(result.begin(), Peer(cfg.network.advertise.netloc(), 0));
    for (const auto &each : result) {
        response->add_peers(each.addr());
    }

    view->update(cfg.peers.c, cfg.peers.H, cfg.peers.S, buffer);
    return grpc::Status::OK;
}

grpc::Status rpc::services::PeersService::list(
    grpc::ServerContext *context,
    const peers::ListRequest *request,
    peers::ListResponse *response
) {
    log("/peers/list", context->peer());

    auto view = _server.peering()->view();
    deque<Peer> snap = view->snapshot();
    for (const auto &each : snap) {
        protos::Peer *peer = response->add_peers();
        peer->set_addr(each.addr());
        peer->set_age(each.age());
    }

    return grpc::Status::OK;
}