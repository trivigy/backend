#include "logging.h"
#include "server/upstream.h"

server::Upstream::Upstream(Server &server) :
    _server(server),
    _timer_info(_ioc, steady_time_point::max()),
    _timer_block_template(_ioc, steady_time_point::max()) {}

void server::Upstream::start() {
    check_info({});
    check_block_template({});

    _handlers.emplace_back([&] { _ioc.run(); });
    server::exit.upstream.get_future().wait();
    _ioc.stop();
    for_each(_handlers.begin(), _handlers.end(), [](thread &t) { t.join(); });
}

void server::Upstream::check_info(error_code code) {
    auto cfg = _server.cfg();
    auto host = cfg.network.upstream.host();
    auto port = cfg.network.upstream.port();

    request<string_body> req;
    req.method(verb::post);
    req.target(string_view(Uri("http", host, port, "/json_rpc").compose()));
    req.set(field::host, string_param(host));
    req.set(field::content_type, string_param("application/javascript"));
    req.set(field::user_agent, string_param(BOOST_BEAST_VERSION_STRING));

    // @formatter:off
    string body = json{
        {"jsonrpc", "2.0"},
        {"id", "0"},
        {"method", "get_info"}
    }.dump();
    // @formatter:on
    req.content_length(body.size());
    req.body() = body;
    req.prepare_payload();

    auto client = make_shared<web::Client>(_ioc);
    client->request(req, [this, client](auto &resp) {
//        cerr << "--- response ---" << endl;
//        cerr << resp << endl;
//        cerr << "--- end ---" << endl;
    }, [this, client](const auto &msg, auto code) {
//        cerr << "--- failure: check_info ---" << endl;
    });

    _timer_info.expires_after(chrono::seconds(1));
    _timer_info.async_wait(
        bind(
            &Upstream::check_info,
            shared_from_this(),
            placeholders::_1
        )
    );
}

void server::Upstream::check_block_template(error_code code) {
    auto cfg = _server.cfg();
    auto host = cfg.network.upstream.host();
    auto port = cfg.network.upstream.port();

    request<string_body> req;
    req.method(verb::post);
    req.target(string_view(Uri("http", host, port, "/json_rpc").compose()));
    req.set(field::host, string_param(host));
    req.set(field::content_type, string_param("application/javascript"));
    req.set(field::user_agent, string_param(BOOST_BEAST_VERSION_STRING));

    // @formatter:off
    string body = json{
        {"jsonrpc", "2.0"},
        {"id", "0"},
        {"method", "getblocktemplate"},
        {"params",
            {
                {"wallet_address", "44GBHzv6ZyQdJkjqZje6KLZ3xSyN1hBSFAnLP6EAqJtCRVzMzZmeXTC2AHKDS9aEDTRKmo6a6o9r9j86pYfhCWDkKjbtcns"},
                {"reserve_size", 60}
            }
        }
    }.dump();
    // @formatter:on
    req.content_length(body.size());
    req.body() = body;
    req.prepare_payload();

    auto client = make_shared<web::Client>(_ioc);
    client->request(req, [this, client](auto &resp) {
//        cerr << "--- response ---" << endl;
//        cerr << resp << endl;
//        cerr << "--- end ---" << endl;
    }, [this, client](const auto &msg, auto code) {
//        cerr << "--- failure: check_block_template ---" << endl;
    });

    _timer_block_template.expires_after(chrono::seconds(1));
    _timer_block_template.async_wait(
        bind(
            &Upstream::check_block_template,
            shared_from_this(),
            placeholders::_1
        )
    );
}