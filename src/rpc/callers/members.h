#ifndef SYNCAIDE_RPC_CALLERS_MEMBERS_H
#define SYNCAIDE_RPC_CALLERS_MEMBERS_H

#include "peer.h"
#include "rpc/helper.h"
#include "protos/members.grpc.pb.h"

#include <grpc++/grpc++.h>
#include <string>
#include <tuple>
#include <deque>

using namespace std;
using namespace protos::members;

namespace rpc {
    namespace callers {
        class MembersCaller {
        public:
            explicit MembersCaller(
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
    }
}

#endif //SYNCAIDE_RPC_CALLERS_MEMBERS_H