#include "logging.h"
#include "server/server.h"

server::exit_t server::exit; // NOLINT

server::Peering::Peering(Server &server) :
    _server(server),
    _view(make_shared<View>()),
    _timer(_ioc, steady_time_point::max()) {
    auto cfg = _server.cfg();

    deque<Peer> buffer;
    for (auto &join : cfg.network.joins) {
        buffer.emplace_back(Peer(join.netloc(), 0));
    }
    _view->update(cfg.members.c, cfg.members.H, cfg.members.S, buffer);
}

void server::Peering::start() {
    grpc::ServerBuilder builder;
    rpc::services::MembersService service(_server);

    builder.AddListeningPort(
        _server.cfg().network.bind.netloc(),
        grpc::InsecureServerCredentials()
    );

    builder.RegisterService((Members::Service *) (&service));
    _rpc = builder.BuildAndStart();

    on_pulse({});

    _handlers.emplace_back([&] { _rpc->Wait(); });
    _handlers.emplace_back([&] { _ioc.run(); });
    server::exit.peering.get_future().wait();
    _rpc->Shutdown();
    _ioc.stop();
    for_each(_handlers.begin(), _handlers.end(), [](thread &t) { t.join(); });
}

shared_ptr<View> server::Peering::view() {
    return _view;
}

void server::Peering::on_pulse(error_code code) {
    auto cfg = _server.cfg();
    if (_view->empty()) {
        _timer.expires_after(chrono::seconds(1));
        _timer.async_wait(bind(&Peering::on_pulse, shared_from_this(), _1));
        return;
    }

    deque<Peer> buffer;
    buffer = _view->select(cfg.members.c / 2 - 1, cfg.members.H);
    buffer.emplace(buffer.begin(), Peer(cfg.network.advertise.netloc(), 0));

    auto peer = _view->random_peer();
    rpc::callers::MembersCaller caller(
        grpc::CreateCustomChannel(
            peer.addr(),
            grpc::InsecureChannelCredentials(),
            grpc::ChannelArguments()
        )
    );

    grpc::Status status;
    tie(status, buffer) = caller.gossip(buffer, cfg.network.advertise.netloc());
    _view->update(cfg.members.c, cfg.members.H, cfg.members.S, buffer);

    if (!status.ok()) {
        json extra = {{"peer", peer.addr()}};
        LOG(error) << logging::add_value("Extra", extra.dump())
                   << status.error_message();
    }

    _timer.expires_after(chrono::seconds(1));
    _timer.async_wait(bind(&Peering::on_pulse, shared_from_this(), _1));
}

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
        cerr << "--- response ---" << endl;
        cerr << resp << endl;
        cerr << "--- end ---" << endl;
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
        cerr << "--- response ---" << endl;
        cerr << resp << endl;
        cerr << "--- end ---" << endl;
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

server::Frontend::Frontend(Server &server) :
    _server(server),
    _router(make_shared<Router>()),
    _ctx(context{context::sslv23}),
    _ioc(io_context{(int) server.cfg().network.threads}) {}

void server::Frontend::start() {
    _router->add(Http::health, "/health");
    _router->add(Http::syncaide_js, "/syncaide[.]js");
    _router->add(Http::syncaide_wasm, "/syncaide[.]wasm");
    _router->add(Http::agent_uid, "/agent/{}", params::string());

#ifndef NDEBUG

    _router->add(Http::syncaide_html, "/syncaide[.]html");

#endif //NDEBUG

    load_http_certificate(_ctx);
    auto address = ip::make_address(_server.cfg().network.frontend.host());
    tcp::endpoint endpoint(address, _server.cfg().network.frontend.port());
    make_shared<Listener>(_ioc, _ctx, endpoint, _router)->run();

    _handlers.reserve(_server.cfg().network.threads);
    for (auto i = _server.cfg().network.threads; i > 0; --i) {
        _handlers.emplace_back([&] { _ioc.run(); });
    }

    auto future = server::exit.frontend.get_future();
    future.wait();
    _ioc.stop();
    for_each(_handlers.begin(), _handlers.end(), [](thread &t) { t.join(); });
}

void server::Frontend::load_http_certificate(asio::ssl::context &ctx) {
    std::string const cert =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
        "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
        "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
        "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
        "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
        "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
        "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
        "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
        "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
        "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
        "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
        "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
        "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
        "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
        "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
        "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
        "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
        "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
        "UEVbkhd5qstF6qWK\n"
        "-----END CERTIFICATE-----\n";

    std::string const key =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
        "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
        "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
        "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
        "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
        "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
        "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
        "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
        "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
        "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
        "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
        "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
        "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
        "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
        "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
        "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
        "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
        "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
        "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
        "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
        "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
        "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
        "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
        "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
        "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
        "pVOUFq5mW8p0zbtfHbjkgxyF\n"
        "-----END PRIVATE KEY-----\n";

    std::string const dh =
        "-----BEGIN DH PARAMETERS-----\n"
        "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
        "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
        "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
        "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
        "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
        "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
        "-----END DH PARAMETERS-----\n";

    ctx.set_password_callback(
        [](std::size_t, boost::asio::ssl::context_base::password_purpose) {
            return "test";
        });

    ctx.set_options(
        asio::ssl::context::default_workarounds |
        asio::ssl::context::no_sslv2 |
        asio::ssl::context::single_dh_use
    );

    ctx.use_certificate_chain(boost::asio::buffer(cert.data(), cert.size()));

    ctx.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        asio::ssl::context::file_format::pem
    );

    ctx.use_tmp_dh(boost::asio::buffer(dh.data(), dh.size()));
}

server::Server::Server(Options &options) :
    _cfg(options),
    _peering(make_shared<Peering>(*this)),
    _upstream(make_shared<Upstream>(*this)),
    _frontend(make_shared<Frontend>(*this)) {}

//shared_ptr<server::Server> server::Server::create(shared_ptr<Options> options) {
//    auto server = shared_ptr<Server>(new Server(options));
//    server->_peering = shared_ptr<Peering>(new Peering(server));
//    server->_upstream = make_shared<Upstream>(server);
//    server->_frontend = make_shared<Frontend>(server);
//    return server;
//}

server::Options &server::Server::cfg() {
    return _cfg;
}

int server::Server::start() {
    _handlers.emplace_back(thread([=] { _peering->start(); }));
    _handlers.emplace_back(thread([=] { _upstream->start(); }));
    _handlers.emplace_back(thread([=] { _frontend->start(); }));

    for (auto &thread : _handlers) {
        thread.join();
    }

    return EXIT_SUCCESS;
}

shared_ptr<server::Peering> server::Server::peering() {
    return _peering;
}

shared_ptr<server::Upstream> server::Server::upstream() {
    return _upstream;
}

shared_ptr<server::Frontend> server::Server::frontend() {
    return _frontend;
}
