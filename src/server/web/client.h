#ifndef SYNCAIDE_SERVER_WEB_H
#define SYNCAIDE_SERVER_WEB_H

#include "server/web/session.h"
#include "common/uri.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <functional>
#include <iostream>

using namespace std;

namespace server {
    namespace web {
        namespace ssl = boost::asio::ssl;
        namespace http = boost::beast::http;
        using boost::asio::io_context;
        using boost::asio::steady_timer;
        using common::Uri;

        class Client {
        private:
            io_context &_ioc;
            ssl::context _ctx;

        public:
            explicit Client(io_context &ioc) :
                _ioc(ioc),
                _ctx(ssl::context::sslv23_client) {};

            template<typename Request, typename Success, typename Failure>
            void request(Request &req, Success &&success, Failure &&failure) {
                Uri uri(string(req.target()));
                req.target(uri.path_ext());
                if (uri.is_tls())
                    make_shared<Session>(
                        _ioc,
                        _ctx,
                        req,
                        forward<Success>(success),
                        forward<Failure>(failure)
                    )->run(uri);
                else
                    make_shared<Session>(
                        _ioc,
                        req,
                        forward<Success>(success),
                        forward<Failure>(failure)
                    )->run(uri);
            };
        };
    }
}

#endif //SYNCAIDE_SERVER_WEB_H