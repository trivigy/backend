#include "logging.h"
#include "server/helper.h"

void server::log(const string &call, error_code &code) {
    json extra = {{"call", "shutdown"}, {"error", code.message()}};
    LOG(error) << logging::add_value("Extra", extra.dump());
    return;
}
