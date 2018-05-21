#include "logging.h"
#include "client/client.h"


client::Client::Client(client::Options &options) :
    members(*this),
    miners(*this),
    _cfg(options) {}

client::Options &client::Client::cfg() {
    return _cfg;
}

int client::Client::start() {
    if (_cfg.cmd == "members") {
        if (_cfg.members.cmd == "list") return members.list();
        else if (_cfg.members.cmd == "status") return members.status();
    } else if (_cfg.cmd == "miners") {
        if (_cfg.miners.cmd == "find") return miners.find();
    }
    return EXIT_FAILURE;
}

int client::Client::Members::list() {
    rpc::callers::MembersCaller caller(
        grpc::CreateCustomChannel(
            _self.cfg().network.host.netloc(),
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    auto[status, buffer] = caller.list();
    if (status.ok()) {
        auto view = json::array();
        for (auto &each : buffer.value()) {
            view.push_back(json({
                {"addr", each.addr()},
                {"age", to_string(each.age())}
            }));
            cout << view.dump() << endl;
        }
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}

int client::Client::Members::status() {
    rpc::callers::MembersCaller caller(
        grpc::CreateCustomChannel(
            _self.cfg().network.host.netloc(),
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    auto[status, buffer] = caller.status("world");
    if (status.ok()) {
        json result = {{"message", buffer.value()}};
        cout << result.dump() << endl;
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}

int client::Client::Miners::find() {
    rpc::callers::MinersCaller caller(
        grpc::CreateCustomChannel(
            _self.cfg().network.host.netloc(),
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    auto[status, buffer] = caller.find(_self.cfg().miners.find.id);
    if (status.ok()) {
        cout << buffer.value().dump() << endl;
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}
