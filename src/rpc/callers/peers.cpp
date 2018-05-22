#include "logging.h"
#include "rpc/callers/peers.h"

rpc::response<deque<Peer>>
rpc::callers::PeersCaller::gossip(
    const deque<Peer> &buffer,
    const string &remove
) {
    grpc::ClientContext context;
    peers::GossipRequest request;
    peers::GossipResponse response;

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

rpc::response<nlohmann::json>
rpc::callers::PeersCaller::list() {
    grpc::ClientContext context;
    peers::ListRequest request;
    peers::ListResponse response;

    grpc::Status status = stub->list(&context, request, &response);

    json result = json::array();
    if (status.ok()) {
        auto peers = response.peers();
        auto it = peers.begin();
        while (it != peers.end()) {
            result.emplace_back(json({
                {"addr", it->addr()},
                {"age", it->age()}
            }));
            it++;
        }
    }
    return {status, result};
}