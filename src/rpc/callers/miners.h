#ifndef SYNCAIDE_RPC_CALLERS_MINERS_H
#define SYNCAIDE_RPC_CALLERS_MINERS_H

#include "protos/miners.grpc.pb.h"
#include "rpc/helper.h"

#include <nlohmann/json.hpp>
#include <string>

using namespace std;
using namespace protos::miners;

namespace rpc {
    namespace callers {
        using nlohmann::json;

        class MinersCaller {
        private:
            unique_ptr<Miners::Stub> stub;

        public:
            explicit MinersCaller(
                const shared_ptr<grpc::ChannelInterface> &channel
            ) : stub(Miners::NewStub(channel)) {}

            response<json> list(const string &id);
        };
    }
}

#endif //SYNCAIDE_RPC_CALLERS_MINERS_H