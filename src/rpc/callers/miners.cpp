#include "logging.h"
#include "rpc/callers/miners.h"

rpc::response<string>
rpc::callers::MinersCaller::find(const string &id) {
    grpc::ClientContext context;
    FindRequest request;
    FindResponse response;

    grpc::Status status = stub->find(&context, request, &response);
    if (status.ok()) {
//        deque<Peer> result;
//        auto peers = response.peers();
//        auto it = peers.begin();
//        while (it != peers.end()) {
//            result.emplace_back(Peer(it->addr(), it->age()));
//            it++;
//        }
//        return {status, result};
    }

    return {status, nullopt};
}
