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
        ("bind,b", po::value<string>()
                ->default_value(defaults.network.bind.netloc())
                ->notifier(bind(&server::Options::on_bind, this, _1)),
            "listen for connections on specified address for internal cluster communications (e.g. 172.20.0.2:8847, [::1]:8847, etc.)")
        ("http,h", po::value<string>()
                ->default_value(defaults.network.http.netloc())
                ->notifier(bind(&server::Options::on_http, this, _1)),
            "address to which syncd listens for http connections (e.g. 172.20.0.2:8080, [::1]:8080, etc.)")
        ("joins,j", po::value<vector<string>>()
                ->multitoken()
                ->default_value(vector<string>{}, string())
                ->notifier(bind(&server::Options::on_joins, this, _1)),
            "connect to cluster through the listed addresses (e.g. 172.20.0.2:8847, [::1]:8847, etc.)")
        ("threads", po::value<unsigned int>(&network.threads)
                ->multitoken()
                ->default_value(defaults.network.threads)
                ->notifier(bind(&server::Options::on_threads, this, _1)),
            "number of threads that http context should allow to run concurrently");

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
        joins.push_back(join);
    }

    // @formatter:off
    json extra = {
        {"network",
            {
                {"advertise", this->network.advertise},
                {"bind", this->network.bind},
                {"http", this->network.http.netloc()},
                {"joins", joins}
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

void server::Options::on_bind(string netloc) {
    boost::regex expr(exprs.netloc);
    boost::smatch what;
    if (boost::regex_search(netloc, what, expr)) {
        string ipv4 = what["ipv4"].str();
        string ipv6 = what["ipv6"].str();
        network.bind = !ipv4.empty() ? ipv4 : "[" + ipv6 + "]";

        string port = what["port"].str();
        if (port.empty()) {
            network.bind += ":" + to_string(defaults.network.bind.port);
        } else {
            auto port_ = static_cast<unsigned short>(stoi(port));
            network.bind += ":" + to_string(port_);
        }
    } else {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "bind");
    }
}

void server::Options::on_http(string netloc) {
    boost::regex expr(exprs.netloc);
    boost::smatch what;
    if (boost::regex_search(netloc, what, expr)) {
        string ipv4 = what["ipv4"].str();
        string ipv6 = what["ipv6"].str();
        network.http.address = !ipv4.empty() ? ipv4 : ipv6;

        string port = what["port"].str();
        if (port.empty()) {
            network.http.port = defaults.network.http.port;
        } else {
            network.http.port = static_cast<unsigned short>(stoi(port));
        }
    } else {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "http");
    }
}

void server::Options::on_joins(vector<string> joins) {
    boost::regex expr(exprs.netloc);
    for (auto &join : joins) {
        boost::smatch what;
        if (boost::regex_search(join, what, expr)) {
            string ipv4 = what["ipv4"].str();
            string ipv6 = what["ipv6"].str();
            string netloc = !ipv4.empty() ? ipv4 : "[" + ipv6 + "]";

            string port = what["port"].str();
            if (port.empty()) {
                netloc += ":" + to_string(defaults.network.bind.port);
                network.joins.emplace_back(netloc);
            } else {
                auto port_ = static_cast<unsigned short>(stoi(port));
                netloc += ":" + to_string(port_);
                network.joins.emplace_back(netloc);
            }
        } else {
            auto kind = po::validation_error::invalid_option_value;
            throw po::validation_error(kind, "joins");
        }
    }
}

void server::Options::on_threads(int threads) {
    if (threads < 1) {
        network.threads = 1;
    } else if (threads > thread::hardware_concurrency()) {
        network.threads = thread::hardware_concurrency();
    }
}
