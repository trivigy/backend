#include "logging.h"
#include "server/server.h"

server::exit_t server::exit; // NOLINT

server::Server::Server(server::Options &options) {
    _cfg = &options;

    deque<Peer> buffer;
    for (auto &join : _cfg->network.joins) {
        buffer.emplace_back(Peer(join, 0));
    }
    _view.update(_cfg->members.c, _cfg->members.H, _cfg->members.S, buffer);
}

server::Options *server::Server::cfg() {
    return _cfg;
}

View *server::Server::view() {
    return &_view;
}

int server::Server::start() {
    _handlers.emplace_back(thread([=] { http_thread(); }));
    _handlers.emplace_back(thread([=] { passive_thread(); }));
    _handlers.emplace_back(thread([=] { active_thread(); }));

    for (auto &thread : _handlers) {
        thread.join();
    }

    return EXIT_FAILURE;
}

void server::Server::http_thread() {
    auto const address = boost::asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    std::string const doc_root = argv[3];
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    // Create and launch a listening port
    std::make_shared<listener>(ioc, ctx, tcp::endpoint{address, port}, doc_root)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc] {
                ioc.run();
            }
        );
    ioc.run();
}

void server::Server::passive_thread() {
    grpc::ServerBuilder builder;
    rpc::MembersService service(this);

    builder.AddListeningPort(
        _cfg->network.bind,
        grpc::InsecureServerCredentials()
    );

//    builder.SetSyncServerOption(ServerBuilder::NUM_CQS, 1);
    builder.RegisterService((Members::Service *) (&service));
    _server = builder.BuildAndStart();
    thread runner_thread([&]() {
        _server->Wait();
    });

    json extra = {{"network", _cfg->network.bind}};
    LOG(info) << logging::add_value("Extra", extra.dump());

    auto future = server::exit.passive.get_future();
    future.wait();
    _server->Shutdown();
    runner_thread.join();
}

void server::Server::active_thread() {
    auto future = server::exit.active.get_future();
    while (future.wait_for(chrono::seconds(1)) != future_status::ready) {
        if (_view.empty())
            continue;

        deque<Peer> buffer;
        buffer = _view.select(_cfg->members.c / 2 - 1, _cfg->members.H);
        buffer.emplace(buffer.begin(), Peer(_cfg->network.advertise, 0));

        auto peer = _view.random_peer();
        rpc::MembersClient client(
            grpc::CreateCustomChannel(
                peer.addr(),
                grpc::InsecureChannelCredentials(),
                grpc::ChannelArguments()
            )
        );

        grpc::Status status;
        tie(status, buffer) = client.gossip(buffer, _cfg->network.advertise);
        _view.update(_cfg->members.c, _cfg->members.H, _cfg->members.S, buffer);

        if (!status.ok()) {
            json extra = {{"peer", peer.addr()}};
            LOG(error) << logging::add_value("Extra", extra.dump())
                       << status.error_message();
        }
    }
}