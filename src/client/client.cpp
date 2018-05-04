#include "logging.h"
#include "client/client.h"

client::Client::Client(client::Options &options) : _cfg(&options) {}

client::Options *client::Client::cfg() {
    return _cfg;
}

int client::Client::start() {
    if (_cfg->cmd == "members") {
        if (_cfg->members.cmd == "gossip") {
            return members_gossip();
        } else if (_cfg->members.cmd == "list") {
            return members_list();
        } else if (_cfg->members.cmd == "status") {
            return members_status();
        }
    }
    return EXIT_FAILURE;
}

int client::Client::members_gossip() {
    rpc::callers::MembersCaller caller(
        grpc::CreateCustomChannel(
            _cfg->network.host,
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    grpc::Status status;
    deque<Peer> buffer;
    tie(status, buffer) = caller.gossip();

    if (status.ok()) {
        auto results = json::array();
        for (auto &each : buffer) {
            results.push_back(each.addr());
        }
        cout << results.dump() << endl;
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}

int client::Client::members_list() {
    rpc::callers::MembersCaller caller(
        grpc::CreateCustomChannel(
            _cfg->network.host,
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    grpc::Status status;
    deque<Peer> buffer;
    tie(status, buffer) = caller.list();

    if (status.ok()) {
        auto view = json::array();
        for (auto &each : buffer) {
            view.push_back(json(
                {
                    {"addr", each.addr()},
                    {"age",  to_string(each.age())}
                }
            ));
            cout << view.dump() << endl;
        }
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}

int client::Client::members_status() {
    rpc::callers::MembersCaller caller(
        grpc::CreateCustomChannel(
            _cfg->network.host,
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    string message("world");
    grpc::Status status;
    string reply;
    tie(status, reply) = caller.status(message);

    if (status.ok()) {
        json result = {{"message", reply}};
        cout << result.dump() << endl;
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}