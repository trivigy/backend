#ifndef SYNCAIDE_RPC_SERVICES_MEMBERS_H
#define SYNCAIDE_RPC_SERVICES_MEMBERS_H

#include "peer.h"
#include "rpc/helper.h"
#include "server/server.h"
#include "protos/members.grpc.pb.h"

#include <string>
#include <tuple>

using namespace std;
using namespace protos::members;

namespace server {
    class Server;
}

namespace rpc {
    namespace services {
        using namespace boost::algorithm;

        class MembersService final : public Members::Service {
        private:
            shared_ptr<server::Server> _server;

        public:
            explicit MembersService(shared_ptr<server::Server> server);

            grpc::Status gossip(
                grpc::ServerContext *context,
                const GossipRequest *request,
                GossipResponse *response
            ) override;

            grpc::Status list(
                grpc::ServerContext *context,
                const ListRequest *request,
                ListResponse *response
            ) override;

            grpc::Status status(
                grpc::ServerContext *context,
                const StatusRequest *request,
                StatusResponse *response
            ) override;
        };
    }
}

#endif //SYNCAIDE_RPC_SERVICES_MEMBERS_H