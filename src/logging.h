#ifndef SYNC_LOGGING_H
#define SYNC_LOGGING_H

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/make_shared.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/fifo_map.hpp>
#include <iostream>

namespace logging = boost::log;
namespace srcs = logging::sources;
namespace exprs = logging::expressions;
namespace sinks = logging::sinks;
namespace attrs = logging::attributes;
namespace keywords = logging::keywords;
namespace trivial = logging::trivial;

using namespace std;
using namespace trivial;

template<class K, class V, class dummy_compare, class A>
using fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;

#define LOG(lvl) BOOST_LOG_SEV(logger::get(), trivial::lvl)

class Logging {
public:
    Logging();

    auto &get();

private:
    srcs::severity_logger_mt<trivial::severity_level> _slg;

    static void formatter(
        logging::record_view const &rec,
        logging::formatting_ostream &strm
    );
};

extern Logging logger;

#endif //SYNC_LOGGING_H