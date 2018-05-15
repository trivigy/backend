#include "logging.h"
#include "server/server.h"

void handler(int) {
    try {
        server::exit.peering.set_value();
        server::exit.upstream.set_value();
        server::exit.frontend.set_value();
    } catch (const future_error &err) {
        LOG(warning) << "something is going on.";
    }
}

int main(int argc, const char **argv) {
    server::Options options;
    if (!options.parse(argc, argv)) {
        return EXIT_FAILURE;
    }

    signal(SIGINT, handler);
    signal(SIGTERM, handler);
    signal(SIGQUIT, handler);

    server::Server server(options);
    return server.start();
}