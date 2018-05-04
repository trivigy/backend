#include "logging.h"
#include "server/options.h"
#include "options.h"

bool server::Options::parse(int argc, const char **argv) {
    using namespace std::placeholders;

    string program_name = program_location().stem().string();
    string description = "Description of this application.";
    vector<po::options_description> descriptors;
    descriptors.emplace_back(po::options_description("Options"));
    descriptors.back().add_options()
        ("help", "show this help message and exit.")
        ("advertise,a", po::value<string>()
                ->default_value(defaults.network.bind.netloc(), "bind")
                ->notifier(bind(&server::Options::on_advertise, this, _1)),
            "used as the address that is advertised to other nodes in the cluster.")
        ("bind,b", po::value<string>()
                ->default_value(defaults.network.bind.netloc())
                ->notifier(bind(&server::Options::on_bind, this, _1)),
            "listen for connections on specified address for internal cluster communications (e.g. 172.20.0.2:8847, [::1]:8847, etc.)")
        ("joins,j", po::value<vector<string>>()
                ->multitoken()
                ->default_value(vector<string>{}, string())
                ->notifier(bind(&server::Options::on_joins, this, _1)),
            "connect to cluster through the listed addresses (e.g. 172.20.0.2:8847, [::1]:8847, etc.)")
        ("upstreams,u", po::value<vector<string>>()
                ->multitoken()
                ->default_value(vector<string>{}, string())
                ->notifier(bind(&server::Options::on_upstreams, this, _1)),
            "designated cryptocurrency clients for connection through the listed addresses (e.g. 172.20.0.2:8847, [::1]:8847, etc.)")
        ("http,h", po::value<string>()
                ->default_value(defaults.network.http.netloc())
                ->notifier(bind(&server::Options::on_http, this, _1)),
            "address to which syncd listens for http connections (e.g. 172.20.0.2:8080, [::1]:8080, etc.)")
        ("threads", po::value<unsigned int>(&network.threads)
                ->multitoken()
                ->default_value(defaults.network.threads)
                ->notifier(bind(&server::Options::on_threads, this, _1)),
            "number of threads that http context should allow to run concurrently.");

    vector<po::positional_options_description> positions;
    positions.emplace_back(po::positional_options_description());

    vector<po::variables_map> maps;
    maps.emplace_back(po::variables_map());

    try {
        po::store(
            po::command_line_parser(argc, argv)
                .options(descriptors.back())
                .positional(positions.back())
                .run(),
            maps.back()
        );
    } catch (exception &e) {
        cerr << "error: " << e.what() << endl;
        cout << usage(
            program_name,
            description,
            descriptors,
            positions,
            maps
        ) << endl;
        return false;
    }

    if (maps.back().count("help")) {
        cout << usage(
            program_name,
            description,
            descriptors,
            positions,
            maps
        ) << endl;
        return false;
    }

    try {
        po::notify(maps.back());
    } catch (exception &e) {
        cerr << "error: " << e.what() << endl;
        return false;
    }

    if (maps.back()["advertise"].as<string>().empty()) {
        network.advertise = network.bind;
    }

    auto joins = json::array();
    for (auto &join : this->network.joins) {
        joins.emplace_back(join.netloc());
    }

    auto upstreams = json::array();
    for (auto &upstream : this->network.upstreams) {
        upstreams.emplace_back(upstream.netloc());
    }

    // @formatter:off
    json extra = {
        {"network",
            {
                {"advertise", this->network.advertise.netloc()},
                {"bind", this->network.bind.netloc()},
                {"joins", joins},
                {"upstreams", upstreams},
                {"http", this->network.http.netloc()}
            }
        },
        {"members",
            {
                {"c", this->members.c},
                {"H", this->members.H},
                {"S", this->members.S}
            }
        }
    };
    // @formatter:on

    LOG(info) << logging::add_value("Extra", extra.dump());
    return true;
}

void server::Options::on_advertise(string source) {
    try {
        network.advertise = Endpoint(source);
    } catch (const exception& e) {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "advertise");
    }
}

void server::Options::on_bind(string source) {
    try {
        network.bind = Endpoint(source);
    } catch (const exception& e) {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "bind");
    }
}

void server::Options::on_joins(vector<string> sources) {
    try {
        for (auto &source : sources) {
            network.joins.emplace_back(Endpoint(source));
        }
    } catch (const exception& e) {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "joins");
    }
}

void server::Options::on_upstreams(vector<string> sources) {
    try {
        for (auto &source : sources) {
            network.upstreams.emplace_back(Endpoint(source));
        }
    } catch (const exception& e) {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "upstreams");
    }
}

void server::Options::on_http(string source) {
    try {
        network.http = Endpoint(source);
    } catch (const exception& e) {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "http");
    }
}

void server::Options::on_threads(int threads) {
    if (threads < 1) {
        network.threads = 1;
    } else if (threads > thread::hardware_concurrency()) {
        network.threads = thread::hardware_concurrency();
    }
}