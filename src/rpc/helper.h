#ifndef SYNCAIDE_RPC_H
#define SYNCAIDE_RPC_H

#include <nlohmann/json.hpp>
#include <grpc++/grpc++.h>
#include <optional>
#include <string>
#include <vector>

using namespace std;

namespace rpc {
    using json = nlohmann::json;

    template <typename Type>
    using response = tuple<grpc::Status, optional<Type>>;

    void log(const string &call, const string &peer);
};

#endif //SYNCAIDE_RPC_H
