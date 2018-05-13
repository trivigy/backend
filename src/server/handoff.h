#ifndef SYNCAIDE_SERVER_HANDOFF_H
#define SYNCAIDE_SERVER_HANDOFF_H

#include "server/http.h"
#include "server/helper.h"
#include "server/router.h"
#include "server/detector.h"
#include "server/server.h"

using namespace std;

namespace server {
    class Server;

    class Handoff : public enable_shared_from_this<Handoff> {
    private:
        context &_ctx;
        Server &_server;
        Router &_router;
        tcp::socket _socket;
        strand<io_context::executor_type> _strand;
        flat_buffer _buffer;

    public:
        explicit Handoff(
            Server &server,
            context &ctx,
            Router &router,
            tcp::socket socket
        );

        void run();

        void on_detect(error_code code, tribool secured);
    };
}

#endif //SYNCAIDE_SERVER_HANDOFF_H