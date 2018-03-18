#ifndef SYNCAIDE_RPC_H
#define SYNCAIDE_RPC_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using namespace std;

namespace rpc {
    using json = nlohmann::json;

    void log(const string &call, const string &peer);
};

#endif //SYNCAIDE_RPC_H
