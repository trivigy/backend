#include "logging.h"
#include "client/client.h"

int main(int argc, const char **argv) {
    client::Options options;
    if (!options.parse(argc, argv)) {
        return EXIT_FAILURE;
    }

    client::Client client(options);
    return client.start();
}