#include "logging.h"
#include "rpc/helper.h"

void rpc::log(const string &msg, const string &peer, const json &details) {
    json result = {{"message", msg}};
    string scheme, netloc = peer;
    const auto pos = peer.find_first_of(':');
    if (string::npos != pos) {
        scheme = peer.substr(0, pos);
        netloc = peer.substr(pos + 1);
    }

    if (scheme != "unix") result["addr"] = netloc;
    else result["addr"] = "unix:socket";
    if (!details.empty()) result["details"] = details;
    LOG(info) << logging::add_value("Extra", result.dump());
}