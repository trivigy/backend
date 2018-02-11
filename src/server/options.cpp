#include "logging.h"
#include "server/options.h"

bool server::Options::parse(int argc, const char **argv) {
    using namespace std::placeholders;

    string program_name = basename(argv[0]);
    string description = "Description of this application.";
    vector<po::options_description> descriptors;
    descriptors.emplace_back(po::options_description("Options"));
    descriptors.back().add_options()
        ("help", "show this help message and exit.")
        ("advertise", po::value<string>(&network.advertise)
             ->default_value(string(), "bind"),
         "used as the address that is advertised to other nodes in the cluster.")
        ("bind,b", po::value<string>(&network.bind)
             ->default_value(defaults.network.bind)
             ->notifier(bind(&server::Options::on_bind, this, _1)),
         "listen for connections on specified address (e.g. 172.20.0.2:8847, [::1]:8847, etc.)")
        ("joins,j", po::value<vector<string>>(&network.joins)
             ->multitoken()
             ->default_value(vector<string>{}, string())
             ->notifier(bind(&server::Options::on_joins, this, _1)),
         "connect to cluster through the listed addresses (e.g. 172.20.0.2:8847, [::1]:8847, etc.)");

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

#ifndef NDEBUG
    auto joins = json::array();
    for (auto &join : this->network.joins) {
        joins.push_back(join);
    }

    json extra = {
        {"network",
            {
                {"advertise", this->network.advertise},
                {"bind", this->network.bind},
                {"joins", joins}
            }
        },
        {"members",
            {
                {"c", to_string(this->members.c)},
                {"H", to_string(this->members.H)},
                {"S", to_string(this->members.S)}
            }
        }
    };

    LOG(debug) << logging::add_value("Extra", extra.dump());
#endif //NDEBUG
    return true;
}

void server::Options::on_bind(string bind) {
    const string unix = "unix:";
    if (!bind.compare(0, unix.size(), unix)) {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "bind");
    }

    const string dns = "dns:";
    if (!bind.compare(0, dns.size(), dns)) {
        size_t pos = dns.size();
        while (bind[pos] == '/') {
            ++pos;
        }
        network.bind = bind.substr(pos);
    }
}

void server::Options::on_joins(vector<string> joins) {
    const string unix = "unix:";
    for (auto &join : joins) {
        if (!join.compare(0, unix.size(), unix)) {
            auto kind = po::validation_error::invalid_option_value;
            throw po::validation_error(kind, "joins");
        }
    }
}
