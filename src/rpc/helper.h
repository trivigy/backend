#ifndef SYNCAIDE_RPC_H
#define SYNCAIDE_RPC_H

#include <nlohmann/json.hpp>
#include <grpc++/grpc++.h>
#include <optional>
#include <string>

using namespace std;

namespace rpc {
    using nlohmann::json;

    template<typename Type>
    using response = tuple<grpc::Status, optional<Type>>;

    void log(
        const string &msg,
        const string &peer,
        const json &details = json()
    );
};

#endif //SYNCAIDE_RPC_H
