#ifndef SYNCAIDE_RPC_SERVICES_MINERS_H
#define SYNCAIDE_RPC_SERVICES_MINERS_H

#include "protos/miners.grpc.pb.h"
#include "server/frontend.h"
#include "rpc/helper.h"

#include <string>
#include <tuple>

using namespace std;
using namespace protos::miners;

namespace server {
    class Server;
}

namespace rpc {
    namespace services {
        class MinersService final : public Miners::Service {
        private:
            server::Server &_server;

        public:
            explicit MinersService(server::Server &server);

            grpc::Status find(
                grpc::ServerContext *context,
                const FindRequest *request,
                FindResponse *response
            ) override;
        };
    }
}

#endif //SYNCAIDE_RPC_SERVICES_MINERS_H