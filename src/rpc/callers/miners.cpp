#include "logging.h"
#include "rpc/callers/miners.h"

rpc::response<nlohmann::json>
rpc::callers::MinersCaller::find(const string &id) {
    grpc::ClientContext context;
    FindRequest request;
    FindResponse response;

    request.set_id(id);
    grpc::Status status = stub->find(&context, request, &response);

    json result = json::array();
    if (status.ok()) {
        auto miners = response.miners();
        auto it = miners.begin();
        while (it != miners.end()) {
            result.emplace_back(json({
                {"id", it->id()}
            }));
            it++;
        }
    }
    return {status, result};
}
