#ifndef SYNCAIDE_RPC_SERVICES_MEMBERS_H
#define SYNCAIDE_RPC_SERVICES_MEMBERS_H

#include "protos/members.grpc.pb.h"
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
        namespace members = protos::members;
        using namespace boost::algorithm;

        class MembersService final : public members::Members::Service {
        private:
            server::Server &_server;

        public:
            explicit MembersService(server::Server &server);

            grpc::Status gossip(
                grpc::ServerContext *context,
                const members::GossipRequest *request,
                members::GossipResponse *response
            ) override;

            grpc::Status list(
                grpc::ServerContext *context,
                const members::ListRequest *request,
                members::ListResponse *response
            ) override;
        };
    }
}

#endif //SYNCAIDE_RPC_SERVICES_MEMBERS_H