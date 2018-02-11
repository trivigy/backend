#ifndef SYNC_RPC_H
#define SYNC_RPC_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace rpc {
    using namespace std;
    using json = nlohmann::json;

    void log(const string &call, const string &peer);
};

#endif //SYNC_RPC_H
