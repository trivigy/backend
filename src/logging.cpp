#include "logging.h"

Logging logger; // NOLINT

Logging::Logging() {
    typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
    boost::shared_ptr<text_sink> sink(new text_sink);

    text_sink::locked_backend_ptr backend = (*(&sink))->locked_backend();

    boost::shared_ptr<ostream> console(&clog, boost::null_deleter());
    (*(&backend))->add_stream(console);

    (*(&sink))->set_formatter(&formatter);

    boost::shared_ptr<logging::core> core = logging::core::get();

    core->add_sink((*(&sink)));

    core->add_global_attribute("TimeStamp", attrs::local_clock());
}

auto &Logging::get() {
    return _slg;
}

void Logging::formatter(
    const logging::record_view &record,
    logging::formatting_ostream &stream
) {
    using namespace srcs;
    using namespace boost::posix_time;
    using json = nlohmann::basic_json<fifo_map>;

    switch (record[logging::trivial::severity].get()) {
        case severity_level::trace:
            stream << "\033[94m";
            break;
        case severity_level::debug:
            stream << "\033[94m";
            break;
        case severity_level::info:
            stream << "\033[0m";
            break;
        case severity_level::warning:
            stream << "\033[93m";
            break;
        case severity_level::error:
            stream << "\033[91m";
            break;
        case severity_level::fatal:
            stream << "\033[101m";
            break;
    }

    auto timestamp = logging::extract<ptime>("TimeStamp", record);
    json entry;
    entry["timestamp"] = to_simple_string(timestamp.get());
    entry["severity"] = to_string(record[logging::trivial::severity].get());

    auto extra = logging::extract<string>("Extra", record);
    if (extra) {
        auto parse = json::parse(extra.get());
        for (json::iterator it = parse.begin(); it != parse.end(); ++it) {
            entry[it.key()] = it.value();
        }
    }

    string message = string(record[exprs::smessage].get());
    if (!message.empty()) {
        entry["message"] = message;
    }

    stream << entry.dump();
    stream << "\033[0m";
}