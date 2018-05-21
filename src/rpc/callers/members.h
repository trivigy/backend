#ifndef SYNCAIDE_RPC_CALLERS_MEMBERS_H
#define SYNCAIDE_RPC_CALLERS_MEMBERS_H

#include "protos/members.grpc.pb.h"
#include "rpc/helper.h"
#include "peer.h"

#include <string>
#include <tuple>
#include <deque>

using namespace std;
using namespace protos::members;

namespace rpc {
    namespace callers {
        class MembersCaller {
        private:
            unique_ptr<Members::Stub> stub;

        public:
            explicit MembersCaller(
                const shared_ptr<grpc::ChannelInterface> &channel
            ) : stub(Members::NewStub(channel)) {}

            response<deque<Peer>> gossip(
                const deque<Peer> &buffer = deque<Peer>(),
                const string &remove = string()
            );

            response<json> list();
        };
    }
}

#endif //SYNCAIDE_RPC_CALLERS_MEMBERS_H