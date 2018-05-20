#ifndef SYNCAIDE_SERVER_FRONTEND_H
#define SYNCAIDE_SERVER_FRONTEND_H

#include "server/router.h"
#include "server/http.h"
#include "server/listener.h"
#include "server/miner.h"

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/io_context.hpp>
#include <nlohmann/json.hpp>
#include <shared_mutex>
#include <string>
#include <mutex>

using namespace std;

namespace server {
    namespace ip = boost::asio::ip;
    using boost::asio::ssl::context;
    using boost::asio::io_context;
    using nlohmann::json;

    class Miner;

    class Server;

    class Frontend : public enable_shared_from_this<Frontend> {
        class Miners {
        private:
            Frontend &_self;
            mutable shared_mutex _mutex;
            map<string, shared_ptr<Miner>> _miners;

        public:
            explicit Miners(Frontend &self) : _self(self) {}

            json find(const string &id = string());

            bool add(const string &id, shared_ptr<Miner> miner);

//            shared_ptr<Miner> pop(const string &uid);
        };

    public:
        Miners miners;

    private:
        context _ctx;
        Router _router;
        io_context _ioc;
        Server &_server;
        vector<thread> _handlers;

    public:
        explicit Frontend(Server &server);

        void start();

    private:
        void load_http_certificate(context &ctx);
    };
}

#endif //SYNCAIDE_SERVER_FRONTEND_H