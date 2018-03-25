#include "logging.h"
#include "rpc/callers/members.h"

tuple<grpc::Status, deque<Peer>>
rpc::callers::MembersCaller::gossip(
    const deque<Peer> &buffer,
    const string &remove
) {
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

tuple<grpc::Status, deque<Peer>>
rpc::callers::MembersCaller::list() {
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

tuple<grpc::Status, string>
rpc::callers::MembersCaller::status(const std::string &message) {
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