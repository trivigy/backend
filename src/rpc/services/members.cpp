#include "logging.h"
#include "rpc/services/members.h"

rpc::services::MembersService::MembersService(server::Server &server) :
    _server(server) {}

grpc::Status rpc::services::MembersService::gossip(
    grpc::ServerContext *context,
    const GossipRequest *request,
    GossipResponse *response
) {
    log("/members/gossip", context->peer());

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
    deque<Peer> result = view->select(cfg.members.c / 2 - 1, cfg.members.H);
    result.emplace(result.begin(), Peer(cfg.network.advertise.netloc(), 0));
    for (const auto &each : result) {
        response->add_peers(each.addr());
    }

    view->update(cfg.members.c, cfg.members.H, cfg.members.S, buffer);
    return grpc::Status::OK;
}

grpc::Status rpc::services::MembersService::list(
    grpc::ServerContext *context,
    const ListRequest *request,
    ListResponse *response
) {
    log("/members/list", context->peer());

    auto view = _server.peering()->view();
    deque<Peer> snap = view->snapshot();
    for (const auto &each : snap) {
        protos::Peer *peer = response->add_peers();
        peer->set_addr(each.addr());
        peer->set_age(each.age());
    }

    return grpc::Status::OK;
}

grpc::Status rpc::services::MembersService::status(
    grpc::ServerContext *context,
    const StatusRequest *request,
    StatusResponse *response
) {
    log("/members/status", context->peer());

    std::string prefix("Hello ");
    response->set_message(prefix + request->message());
    return grpc::Status::OK;
}