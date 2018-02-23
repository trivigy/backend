#ifndef SYNC_RPC_MEMBERS_H
#define SYNC_RPC_MEMBERS_H

#include "peer.h"
#include "rpc/rpc.h"
#include "server/server.h"
#include "protos/members.grpc.pb.h"

#include <grpc++/grpc++.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

using namespace std;
using namespace protos::members;

namespace server {
    class Server;
}

namespace rpc {
    using namespace boost::algorithm;

    class MembersClient {
    public:
        explicit MembersClient(
            const shared_ptr<grpc::ChannelInterface> &channel
        ) : stub(Members::NewStub(channel)) {}

        tuple<grpc::Status, deque<Peer>> gossip(
            const deque<Peer> &buffer = deque<Peer>(),
            const string &remove = string()
        );

        tuple<grpc::Status, deque<Peer>> list();

        tuple<grpc::Status, string> status(const string &message);

    private:
        unique_ptr<Members::Stub> stub;
    };

    class MembersService final : public Members::Service {
    public:
        explicit MembersService(server::Server *server);

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

    private:
        server::Server *server;
    };
}

#endif //SYNC_RPC_MEMBERS_H