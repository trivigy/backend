#ifndef SYNCAIDE_RPC_CALLERS_PEERS_H
#define SYNCAIDE_RPC_CALLERS_PEERS_H

#include "protos/peers.grpc.pb.h"
#include "rpc/helper.h"
#include "peer.h"

#include <string>
#include <tuple>
#include <deque>

using namespace std;

namespace rpc {
    namespace callers {
        namespace peers = protos::peers;

        class PeersCaller {
        private:
            unique_ptr<peers::Peers::Stub> stub;

        public:
            explicit PeersCaller(
                const shared_ptr<grpc::ChannelInterface> &channel
            ) : stub(peers::Peers::NewStub(channel)) {}

            response<deque<Peer>> gossip(
                const deque<Peer> &buffer = deque<Peer>(),
                const string &remove = string()
            );

            response<json> list();
        };
    }
}

#endif //SYNCAIDE_RPC_CALLERS_PEERS_H