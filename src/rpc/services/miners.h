#ifndef SYNCAIDE_RPC_SERVICES_MINERS_H
#define SYNCAIDE_RPC_SERVICES_MINERS_H

#include "protos/miners.grpc.pb.h"
#include "server/frontend.h"
#include "rpc/helper.h"

#include <string>
#include <tuple>

using namespace std;

namespace server {
    class Server;
}

namespace rpc {
    namespace services {
        namespace miners = protos::miners;

        class MinersService final : public miners::Miners::Service {
        private:
            server::Server &_server;

        public:
            explicit MinersService(server::Server &server);

            grpc::Status list(
                grpc::ServerContext *context,
                const miners::ListRequest *request,
                miners::ListResponse *response
            ) override;
        };
    }
}

#endif //SYNCAIDE_RPC_SERVICES_MINERS_H