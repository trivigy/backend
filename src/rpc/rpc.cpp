#include "logging.h"
#include "rpc/rpc.h"

void rpc::log(const string &call, const string &peer) {
    json result = {{"call", call}};
    string scheme, netloc = peer;
    const auto pos = peer.find_first_of(':');
    if (string::npos != pos) {
        scheme = peer.substr(0, pos);
        netloc = peer.substr(pos + 1);
    }

    const string unix = "unix";
    if (scheme.compare(0, unix.size(), unix)) {
        result["addr"] = netloc;
    }
    LOG(debug) << logging::add_value("Extra", result.dump());
}