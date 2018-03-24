#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <fmt/format.h>
#include <iostream>

using namespace std;
namespace fs = boost::filesystem;
using boost::dll::program_location;

class args {
public:
    explicit args(int argc, const char **argv) :
        _argc(argc - 1),
        _argv(argv + 1) {}

    const char *const *begin() const { return _argv; }

    const char *const *end() const { return _argv + _argc; }

private:
    const int _argc;
    const char **_argv;
};

int main(int argc, const char **argv) {
    if (argc < 2) {
        string program_name = program_location().stem().string();
        cerr << fmt::format(
            "usage: {} <filename> [<filename>, ...]",
            program_name
        ) << endl;
        return EXIT_FAILURE;
    }

    ofstream ofs("resources.cpp");
    ofs << "#include <map>" << endl;
    ofs << "#include <vector>" << endl;
    ofs << "#include <string>" << endl;
    ofs << endl;
    ofs << "using namespace std;" << endl;
    ofs << endl;
    ofs << "map<string, vector<unsigned char>> resources{" << endl;

    for (const char *each : args(argc, argv)) {
        fs::path filepath(fs::canonical(fs::path(each)));
        string filename = filepath.filename().string();
        ofs << fmt::format("{{\"{0}\", {{", filename) << endl;

        ifstream ifs(filepath.string(), ios::binary);
        ifs.unsetf(ios::skipws);
        ifs.seekg(0, ios::end);
        streampos pos = ifs.tellg();
        ifs.seekg(0, ios::beg);

        vector<unsigned char> data;
        data.reserve((size_t) pos);
        data.insert(data.begin(),
            istream_iterator<unsigned char>(ifs),
            istream_iterator<unsigned char>()
        );

        unsigned int count = 0;
        for (auto &byte : data) {
            count = (count + 1) % 10;
            ofs << fmt::format("0x{:0=2x}, ", byte);
            if (count == 0) {
                ofs << endl;
            }
        }

        ofs << "}}," << endl;
    }

    ofs << "};" << endl;
    return EXIT_SUCCESS;
}