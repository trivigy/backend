#include "logging.h"
#include "rpc/members.h"

rpc::MembersService::MembersService(server::Server *server) {
    this->server = server;
}

tuple<grpc::Status, deque<Peer>>
rpc::MembersClient::gossip(const deque<Peer> &buffer, const string &remove) {
    grpc::ClientContext context;
    GossipRequest request;
    GossipResponse response;

    for (const auto &each : buffer) {
        request.add_peers(each.addr());
    }

    deque<Peer> result;
    grpc::Status status = stub->gossip(&context, request, &response);
    if (status.ok()) {
        auto peers = response.peers();
        auto it = peers.begin();
        while (it != peers.end()) {
            auto peer = *it;
            if (remove.compare(0, peer.size(), peer)) {
                result.emplace_back(Peer(peer, 0));
            }
            it++;
        }
    }

    return {status, result};
}

grpc::Status rpc::MembersService::gossip(
    grpc::ServerContext *context,
    const GossipRequest *request,
    GossipResponse *response
) {
    log("/members/gossip", context->peer());

    deque<Peer> buffer;
    auto cfg = server->cfg();
    auto peers = request->peers();
    auto it = peers.begin();
    while (it != peers.end()) {
        auto peer = *it;
        if (cfg->network.advertise.compare(0, peer.size(), peer)) {
            buffer.emplace_back(Peer(peer, 0));
        }
        it++;
    }

    auto view = server->view();
    deque<Peer> result = view->select(cfg->members.c / 2 - 1, cfg->members.H);
    result.emplace(result.begin(), Peer(cfg->network.advertise, 0));
    for (const auto &each : result) {
        response->add_peers(each.addr());
    }

    view->update(cfg->members.c, cfg->members.H, cfg->members.S, buffer);
    return grpc::Status::OK;
}

tuple<grpc::Status, deque<Peer>> rpc::MembersClient::list() {
    grpc::ClientContext context;
    ListRequest request;
    ListResponse response;

    deque<Peer> result;
    grpc::Status status = stub->list(&context, request, &response);
    if (status.ok()) {
        auto peers = response.peers();
        auto it = peers.begin();
        while (it != peers.end()) {
            result.emplace_back(Peer(it->addr(), it->age()));
            it++;
        }
    }

    return {status, result};
}

grpc::Status rpc::MembersService::list(
    grpc::ServerContext *context,
    const ListRequest *request,
    ListResponse *response
) {
    log("/members/list", context->peer());

    auto view = server->view();
    deque<Peer> snap = view->snapshot();
    for (const auto &each : snap) {
        protos::Peer *peer = response->add_peers();
        peer->set_addr(each.addr());
        peer->set_age(each.age());
    }

    return grpc::Status::OK;
}

tuple<grpc::Status, string>
rpc::MembersClient::status(const std::string &message) {
    grpc::ClientContext context;
    StatusRequest request;
    StatusResponse response;

    string result;
    request.set_message(message);
    grpc::Status status = stub->status(&context, request, &response);
    if (status.ok()) {
        result = response.message();
    }

    return {status, result};
}

grpc::Status rpc::MembersService::status(
    grpc::ServerContext *context,
    const StatusRequest *request,
    StatusResponse *response
) {
    log("/members/status", context->peer());

    std::string prefix("Hello ");
    response->set_message(prefix + request->message());
    return grpc::Status::OK;
}