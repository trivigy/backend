#include "logging.h"
#include "client/client.h"


client::Client::Client(client::Options &options) :
    peers(*this),
    miners(*this),
    _cfg(options) {}

client::Options &client::Client::cfg() {
    return _cfg;
}

int client::Client::start() {
    if (_cfg.cmd == "peers") {
        if (_cfg.peers.cmd == "list") return peers.list();
    } else if (_cfg.cmd == "miners") {
        if (_cfg.miners.cmd == "list") return miners.list();
    }
    return EXIT_FAILURE;
}

int client::Client::Peers::list() {
    rpc::callers::PeersCaller caller(
        grpc::CreateCustomChannel(
            _self.cfg().network.host.netloc(),
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    auto[status, buffer] = caller.list();
    if (status.ok()) {
        cout << buffer.value().dump() << endl;
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}

int client::Client::Miners::list() {
    rpc::callers::MinersCaller caller(
        grpc::CreateCustomChannel(
            _self.cfg().network.host.netloc(),
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    auto[status, buffer] = caller.list(_self.cfg().miners.list.id);
    if (status.ok()) {
        cout << buffer.value().dump() << endl;
    } else {
        LOG(error) << status.error_message();
    }

    return status.error_code();
}
