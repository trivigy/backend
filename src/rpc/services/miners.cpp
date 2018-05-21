#include "logging.h"
#include "rpc/services/miners.h"

rpc::services::MinersService::MinersService(server::Server &server) :
    _server(server) {}

grpc::Status rpc::services::MinersService::list(
    grpc::ServerContext *context,
    const miners::ListRequest *request,
    miners::ListResponse *response
) {
    json details;
    if (!request->id().empty()) details["id"] = request->id();
    log("/miners/find", context->peer(), details);

    auto miners = _server.frontend()->miners.list(request->id());
    for (const auto &each : miners) {
        protos::Miner *miner = response->add_miners();
        miner->set_id(each->id());
    }
    return grpc::Status::OK;
}