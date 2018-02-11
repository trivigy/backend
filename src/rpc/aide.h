#ifndef SYNC_RPC_AIDE_H
#define SYNC_RPC_AIDE_H

#include "stubs/aide.server.h"

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <json/json.h>
#include <iostream>

using namespace std;

namespace rpc {
    using namespace jsonrpc;

    class AideService : public stubs::aide::AbstractStubServer {
    public:
        explicit AideService(AbstractServerConnector &connector);

        string submit(const string &name) override;
    };
}

#endif //SYNC_RPC_AIDE_H
