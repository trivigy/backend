#ifndef SYNCAIDE_SERVER_HELPER_H
#define SYNCAIDE_SERVER_HELPER_H

#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>

using namespace std;

namespace server {
    using nlohmann::json;
    using boost::system::error_code;

    typedef chrono::time_point<chrono::steady_clock> steady_time_point;

    void log(const string &call, error_code &code);
}

#endif //SYNCAIDE_SERVER_HELPER_H
