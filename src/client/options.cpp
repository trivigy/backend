#include "logging.h"
#include "client/options.h"

bool client::Options::parse(int argc, const char **argv) {
    using namespace std::placeholders;

    string program_name = program_location().stem().string();
    string description = "This works.";
    vector<po::options_description> descriptors;
    descriptors.emplace_back(po::options_description("Options"));
    descriptors.back().add_options()
        ("help", "show this help message and exit")
        ("host,h", po::value<string>(&network.host)
                ->default_value(defaults.network.host)
                ->notifier(bind(&client::Options::on_host, this, _1)),
            "connect to node at the specified hostname (e.g. 172.20.0.2:8847, [::1]:8847, etc.)")
        ("cmd", po::value<string>(&cmd)->default_value(""));

    vector<po::positional_options_description> positions;
    positions.emplace_back(po::positional_options_description());
    positions.back().add("cmd", 1).add("args", -1);

    vector<po::options_description> commands;
    commands.emplace_back(po::options_description("Commands"));
    commands.back().add_options()
        ("members", "members command");

    vector<po::variables_map> maps;
    maps.emplace_back(po::variables_map());
    po::parsed_options parsed = po::command_line_parser(argc, argv)
        .options(descriptors.back())
        .positional(positions.back())
        .allow_unregistered()
        .run();

    vector<vector<string>> options;
    options.emplace_back(vector<string>());
    auto it = parsed.options.begin();
    while (it != parsed.options.end()) {
        auto begin = it->original_tokens.begin();
        auto end = it->original_tokens.end();
        copy(begin, end, back_inserter(options.back()));
        if (it->position_key != -1) {
            it++;
            break;
        }
        it++;
    }

    try {
        po::store(
            po::command_line_parser(options.back())
                .options(descriptors.back())
                .positional(positions.back())
                .allow_unregistered()
                .run(),
            maps.back()
        );
    } catch (exception &e) {
        cerr << "\033[1;91m" << "error: " << e.what() << "\033[0m" << endl;
        cout << usage(
            program_name,
            description,
            descriptors,
            positions,
            maps,
            commands.back()
        ) << endl;
        return false;
    }

    if (maps.back().count("help")) {
        cout << usage(
            program_name,
            description,
            descriptors,
            positions,
            maps,
            commands.back()
        ) << endl;
        return false;
    }

    string cmd = maps.back()["cmd"].as<string>();
    if (cmd == "members") {
        program_name += " " + cmd;
        description = "This is how it works.";
        descriptors.emplace_back(po::options_description("Members Options"));
        descriptors.back().add_options()
            ("help", "show this help message and exit.")
            ("cmd", po::value<string>(&members.cmd)->default_value(""));

        positions.emplace_back(po::positional_options_description());
        positions.back().add("cmd", 1).add("args", -1);

        commands.emplace_back(po::options_description("Commands"));
        commands.back().add_options()
            ("gossip", "gossip command")
            ("list", "list command")
            ("status", "status command");

        options.emplace_back(vector<string>());
        while (it != parsed.options.end()) {
            auto begin = it->original_tokens.begin();
            auto end = it->original_tokens.end();
            copy(begin, end, back_inserter(options.back()));
            if (it->position_key != -1) {
                it++;
                break;
            }
            it++;
        }

        maps.emplace_back(po::variables_map());
        try {
            po::store(
                po::command_line_parser(options.back())
                    .options(descriptors.back())
                    .positional(positions.back())
                    .allow_unregistered()
                    .run(),
                maps.back()
            );
        } catch (exception &e) {
            cerr << "\033[1;91m" << "error: " << e.what() << "\033[0m" << endl;
            cout << usage(
                program_name,
                description,
                descriptors,
                positions,
                maps,
                commands.back()
            ) << endl;
            return false;
        }

        if (maps.back().count("help")) {
            cout << usage(
                program_name,
                description,
                descriptors,
                positions,
                maps,
                commands.back()
            ) << endl;
            return false;
        }

        cmd.clear();
        cmd = maps.back()["cmd"].as<string>();
        if (cmd == "gossip") {
            program_name += " " + cmd;
            description = "This is how it works.";
            descriptors.emplace_back(po::options_description("Gossip Options"));
            descriptors.back().add_options()
                ("help", "show this help message and exit.");

            positions.emplace_back(po::positional_options_description());

            options.emplace_back(vector<string>());
            while (it != parsed.options.end()) {
                auto begin = it->original_tokens.begin();
                auto end = it->original_tokens.end();
                copy(begin, end, back_inserter(options.back()));
                it++;
            }

            maps.emplace_back(po::variables_map());
            try {
                po::store(
                    po::command_line_parser(options.back())
                        .options(descriptors.back())
                        .positional(positions.back())
                        .run(),
                    maps.back()
                );
            } catch (exception &e) {
                cerr << "\033[1;91m"
                     << "error: " << e.what()
                     << "\033[0m" << endl;
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

            for (auto &vm : maps) {
                try {
                    po::notify(vm);
                } catch (exception &e) {
                    cerr << "error: " << e.what() << endl;
                    return false;
                }
            }

            return true;
        } else if (cmd == "list") {
            program_name += " " + cmd;
            description = "This is how it works.";
            descriptors.emplace_back(po::options_description("List Options"));
            descriptors.back().add_options()
                ("help", "show this help message and exit.");

            positions.emplace_back(po::positional_options_description());

            options.emplace_back(vector<string>());
            while (it != parsed.options.end()) {
                auto begin = it->original_tokens.begin();
                auto end = it->original_tokens.end();
                copy(begin, end, back_inserter(options.back()));
                it++;
            }

            maps.emplace_back(po::variables_map());
            try {
                po::store(
                    po::command_line_parser(options.back())
                        .options(descriptors.back())
                        .positional(positions.back())
                        .run(),
                    maps.back()
                );
            } catch (exception &e) {
                cerr << "\033[1;91m"
                     << "error: " << e.what()
                     << "\033[0m" << endl;
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

            for (auto &vm : maps) {
                try {
                    po::notify(vm);
                } catch (exception &e) {
                    cerr << "error: " << e.what() << endl;
                    return false;
                }
            }

            return true;
        } else if (cmd == "status") {
            program_name += " " + cmd;
            description = "This is how it works.";
            descriptors.emplace_back(po::options_description("Status Options"));
            descriptors.back().add_options()
                ("help", "show this help message and exit.");

            positions.emplace_back(po::positional_options_description());

            options.emplace_back(vector<string>());
            while (it != parsed.options.end()) {
                auto begin = it->original_tokens.begin();
                auto end = it->original_tokens.end();
                copy(begin, end, back_inserter(options.back()));
                it++;
            }

            maps.emplace_back(po::variables_map());
            try {
                po::store(
                    po::command_line_parser(options.back())
                        .options(descriptors.back())
                        .positional(positions.back())
                        .run(),
                    maps.back()
                );
            } catch (exception &e) {
                cerr << "\033[1;91m"
                     << "error: " << e.what()
                     << "\033[0m" << endl;
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

            for (auto &vm : maps) {
                try {
                    po::notify(vm);
                } catch (exception &e) {
                    cerr << "error: " << e.what() << endl;
                    return false;
                }
            }

            return true;
        } else if (cmd.empty()) {
            cerr << "\033[1;91m"
                 << "error: unspecified command"
                 << "\033[0m" << endl;
            cout << usage(
                program_name,
                description,
                descriptors,
                positions,
                maps,
                commands.back()
            ) << endl;
            return false;
        } else {
            using boost::format;
            cerr << "\033[1;91m"
                 << format("error: unrecognised command '%1%'") % cmd
                 << "\033[0m" << endl;
            cout << usage(
                program_name,
                description,
                descriptors,
                positions,
                maps,
                commands.back()
            ) << endl;
            return false;
        }
    } else if (cmd.empty()) {
        cerr << "\033[1;91m"
             << "error: unspecified command"
             << "\033[0m" << endl;
        cout << usage(
            program_name,
            description,
            descriptors,
            positions,
            maps,
            commands.back()
        ) << endl;
        return false;
    } else {
        using boost::format;
        cerr << "\033[1;91m"
             << format("error: unrecognised command '%1%'") % cmd
             << "\033[0m" << endl;
        cout << usage(
            program_name,
            description,
            descriptors,
            positions,
            maps,
            commands.back()
        ) << endl;
        return false;
    }

    for (auto &vm : maps) {
        try {
            po::notify(vm);
        } catch (exception &e) {
            cerr << "\033[1;91m" << "error: " << e.what() << "\033[0m" << endl;
            return false;
        }
    }

    return true;
}

void client::Options::on_host(string host) {
    const string unix = "unix:";
    if (!host.compare(0, unix.size(), unix)) {
        auto kind = po::validation_error::invalid_option_value;
        throw po::validation_error(kind, "host");
    }
}
