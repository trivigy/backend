#ifndef SYNCAIDE_SERVER_HANDOFF_H
#define SYNCAIDE_SERVER_HANDOFF_H

#include "server/http.h"
#include "server/helper.h"
#include "server/router.h"
#include "server/detector.h"

using namespace std;

namespace server {
    class Handoff : public enable_shared_from_this<Handoff> {
    private:
        tcp::socket _socket;
        context &_ctx;
        strand<io_context::executor_type> _strand;
        shared_ptr<Router> _router;
        flat_buffer _buffer;

    public:
        explicit Handoff(
            tcp::socket socket,
            context &ctx,
            shared_ptr<Router> router
        );

        void run();

        void on_detect(error_code code, tribool secured);
    };
}

#endif //SYNCAIDE_SERVER_HANDOFF_H