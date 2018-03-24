#include "options.h"

string common::Options::Options::usage(
    const string &program_name,
    string description,
    vector<po::options_description> &descriptors,
    vector<po::positional_options_description> &positions,
    vector<po::variables_map> &maps,
    po::options_description commands
) {
    ostringstream oss;
    vector<string> parts;

    // Usage
    parts.emplace_back("usage: ");
    parts.emplace_back(program_name);
    auto desc = descriptors.back();
    for (const auto &j : desc.options()) {
        string format_short = j->canonical_display_name(
            cls::allow_dash_for_short
        );
        string format_long = j->canonical_display_name(
            cls::allow_long
        );

        if (j->long_name() != "cmd" && j->long_name() != "args") {
            if (j->long_name() != format_short) {
                if (!j->long_name().empty()) {
                    string format_name = format_short;
                    format_name += ",";
                    format_name += format_long;
                    parts.emplace_back('[' + format_name + ']');
                } else {
                    parts.emplace_back('[' + format_short + ']');
                }
            } else {
                parts.emplace_back('[' + format_long + ']');
            }
        }
    }

    if (!commands.options().empty()) {
        parts.emplace_back("COMMAND");
    }

    parts.emplace_back("\n");
    copy(parts.begin(), parts.end(), ostream_iterator<string>(oss, " "));

    // Description
    if (!description.empty()) {
        oss << endl << description << endl;
    }

    // Options
    if (!desc.options().empty()) {
        int max_short = 0;
        int max_long = 0;
        int max_default = 0;
        for (const auto &j : desc.options()) {
            string format_short = j->canonical_display_name(
                cls::allow_dash_for_short
            );
            string format_long = j->canonical_display_name(
                cls::allow_long
            );
            const string _default = j->format_parameter();
            regex rgx("[(]=(.+)[)]");
            smatch match;
            if (regex_search(_default.begin(), _default.end(), match, rgx)) {
                if (!match.empty()) {
                    if (match[1].length() > max_default) {
                        max_default = (int) match[1].length();
                    }
                }
            }

            if (j->long_name() != "cmd" && j->long_name() != "args") {
                if (j->long_name() != format_short) {
                    if (format_short.length() > max_short) {
                        max_short = (int) format_short.length();
                    }
                }

                if (format_long.length() > max_long) {
                    max_long = (int) format_long.length();
                }
            }
        }

        oss << endl << "Options:" << endl;
        for (const auto &j : desc.options()) {
            string format_short = j->canonical_display_name(
                cls::allow_dash_for_short
            );
            string format_long = j->canonical_display_name(
                cls::allow_long
            );

            if (j->long_name() != "cmd" && j->long_name() != "args") {
                oss << "  ";
                if (max_short) {
                    if (j->long_name() != format_short) {
                        if (!j->long_name().empty()) {
                            oss << left
                                << setw(max_short + 1)
                                << format_short + ",";
                        } else {
                            oss << left
                                << setw(max_short + 1)
                                << format_short;
                        }
                    } else {
                        oss << left << setw(max_short + 1) << "";
                    }
                }
                if (!j->long_name().empty()) {
                    oss << left << setw(max_long + 3) << format_long;
                } else {
                    oss << left << setw(max_long + 3) << "";
                }

                const string _default = j->format_parameter();
                regex rgx("[(]=(.+)[)]");
                smatch match;
                regex_search(_default.begin(), _default.end(), match, rgx);
                if (max_default) {
                    if (!match.empty()) {
                        oss << left << setw(max_default + 5)
                            << '[' + match[1].str() + ']';
                    } else {
                        oss << left << setw(max_default + 5) << "";
                    }
                }

                oss << j->description()
                    << endl;
            }
        }
    }

    // Commands
    if (!commands.options().empty()) {
        int max_long_command = 0;
        for (const auto &j : commands.options()) {
            if (j->long_name().length() > max_long_command) {
                max_long_command = (int) j->long_name().length();
            }
        }

        oss << endl << "Commands:" << endl;
        for (const auto &j : commands.options()) {
            oss << "  ";
            oss << left << setw(max_long_command + 3) << j->long_name()
                << j->description()
                << endl;
        }
    }

    return oss.str();
}