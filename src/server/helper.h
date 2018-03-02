#ifndef SYNC_SERVER_HELPER_H
#define SYNC_SERVER_HELPER_H

#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>
#include <string>

using namespace std;

namespace server {
    using nlohmann::json;
    using boost::system::error_code;

    void log(const string &call, error_code &code);
}

#endif //SYNC_SERVER_HELPER_H
