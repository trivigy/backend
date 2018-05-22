#ifndef SYNCAIDE_RPC_SERVICES_PEERS_H
#define SYNCAIDE_RPC_SERVICES_PEERS_H

#include "protos/peers.grpc.pb.h"
#include "server/peering.h"
#include "rpc/helper.h"
#include "peer.h"

#include <string>
#include <tuple>

using namespace std;

namespace server {
    class Server;
}

namespace rpc {
    namespace services {
        namespace peers = protos::peers;
        using namespace boost::algorithm;

        class PeersService final : public peers::Peers::Service {
        private:
            server::Server &_server;

        public:
            explicit PeersService(server::Server &server);

            grpc::Status gossip(
                grpc::ServerContext *context,
                const peers::GossipRequest *request,
                peers::GossipResponse *response
            ) override;

            grpc::Status list(
                grpc::ServerContext *context,
                const peers::ListRequest *request,
                peers::ListResponse *response
            ) override;
        };
    }
}

#endif //SYNCAIDE_RPC_SERVICES_PEERS_H