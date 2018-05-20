#include "logging.h"
#include "server/miner.h"

server::Miner::Miner(
    Server &server,
    context &ctx,
    ssl_stream<tcp::socket> socket,
    boost::tribool secured,
    const string &id
) : _server(server),
    _ctx(ctx),
    _strand(socket.next_layer().get_executor().context().get_executor()),
    _timer(
        socket.next_layer().get_executor().context(),
        steady_time_point::max()
    ),
    _socket(ssl_socket(move(socket))),
    _secured(secured),
    _id(id) {}

server::Miner::Miner(
    Server &server,
    context &ctx,
    tcp::socket socket,
    tribool secured,
    const string &id
) : _server(server),
    _ctx(ctx),
    _strand(socket.get_executor().context().get_executor()),
    _timer(socket.get_executor().context(), steady_time_point::max()),
    _socket(plain_socket(move(socket))),
    _secured(secured),
    _id(id) {}

const string &server::Miner::id() {
    return _id;
}

void server::Miner::run(request_type &&req) {
    on_timer({});
    accept(move(req));
}

void server::Miner::accept(request_type &&req) {
    _timer.expires_after(chrono::seconds(15));
    if (_secured) {
        boost::get<ssl_socket>(_socket).async_accept(
            req,
            bind_executor(
                _strand,
                bind(
                    &Miner::on_accept,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    } else {
        boost::get<plain_socket>(_socket).async_accept(
            req,
            bind_executor(
                _strand,
                bind(
                    &Miner::on_accept,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    }
}

void server::Miner::on_accept(error_code code) {
    if (code == operation_aborted) return;
    if (code) return log("accept", code);
    read();
}

void server::Miner::read() {
    _timer.expires_after(chrono::seconds(15));
    if (_secured) {
        boost::get<ssl_socket>(_socket).async_read(
            _buffer,
            bind_executor(
                _strand,
                bind(
                    &Miner::on_read,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    } else {
        boost::get<plain_socket>(_socket).async_read(
            _buffer,
            bind_executor(
                _strand,
                bind(
                    &Miner::on_read,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    }
}

void server::Miner::on_read(error_code code, size_t bytes_transferred) {
    ignore_unused(bytes_transferred);
    if (code == operation_aborted) return;
    if (code == websocket::error::closed) return;
    if (code) log("read", code);

    protos::Message msg;
    msg.ParseFromString(buffers_to_string(_buffer.data()));
    switch (msg.type()) {
        case protos::MessageType::LOGIN:
            login(msg);
            break;
        case protos::MessageType::TEMPLATE:
            cerr << "protos::MessageType::TEMPLATE" << endl;
            break;
        case protos::MessageType::SUBMIT:
            cerr << "protos::MessageType::SUBMIT" << endl;
            break;
        case protos::MessageType::REPLY:
            cerr << "protos::MessageType::REPLY" << endl;
            break;
        default:
            break;
    }

    _buffer.consume(_buffer.size());

//    protos::Peer outgoing;
//    outgoing.set_addr("2.2.2.2");
//    outgoing.set_age(22);
//    write(buffer(outgoing.SerializeAsString()));
}

void server::Miner::write(const BOOST_ASIO_CONST_BUFFER &buffer) {
    if (_secured) {
        auto &socket = boost::get<ssl_socket>(_socket);
        socket.async_write(
            buffer,
            bind_executor(
                _strand,
                bind(
                    &Miner::on_write,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    } else {
        auto &socket = boost::get<plain_socket>(_socket);
        socket.async_write(
            buffer,
            bind_executor(
                _strand,
                bind(
                    &Miner::on_write,
                    shared_from_this(),
                    placeholders::_1,
                    placeholders::_2
                )
            )
        );
    }
}

void server::Miner::on_write(error_code code, size_t bytes_transferred) {
    ignore_unused(bytes_transferred);
    if (code == operation_aborted) return;
    if (code) return log("write", code);
    read();
}

void server::Miner::on_timer(error_code code) {
    if (code && code != operation_aborted) return log("timer", code);
    if (_timer.expiry() <= chrono::steady_clock::now()) timeout();
    _timer.async_wait(
        bind_executor(
            _strand,
            bind(
                &Miner::on_timer,
                shared_from_this(),
                placeholders::_1
            )
        )
    );
}

void server::Miner::timeout() {
    if (_secured) {
        if (_eof) return;
        _timer.expires_at(steady_time_point::max());

        on_timer({});
        _eof = true;
        _timer.expires_after(chrono::seconds(15));
        boost::get<ssl_socket>(_socket).next_layer().async_shutdown(
            bind_executor(
                _strand,
                bind(
                    &Miner::on_conclude,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    } else {
        if (_close) return;
        _close = true;
        _timer.expires_after(chrono::seconds(15));
        boost::get<plain_socket>(_socket).async_close(
            (const uint16_t) websocket::close_code::normal,
            bind_executor(
                _strand,
                bind(
                    &Miner::on_conclude,
                    shared_from_this(),
                    placeholders::_1
                )
            )
        );
    }
}

void server::Miner::on_conclude(error_code code) {
    if (code == operation_aborted) return;
    if (code) return log("shutdown", code);
}

void server::Miner::login(protos::Message &msg) {
    cerr << "protos::MessageType::LOGIN" << endl;
    const auto &body = msg.login();
//    cerr << "signature: " << body.signature() << endl;
//    cerr << "parameters: " << body.parameters() << endl;

    string parameters_digest;
    CryptoPP::SHA256 sha256sum;
    CryptoPP::StringSource(
        body.parameters(), true,
        new CryptoPP::HashFilter(
            sha256sum,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(parameters_digest)
            )
        )
    );
    to_lower(parameters_digest);

    // TODO verify signature with parameters_digest and private key

    _server.frontend()->miners.add(_id, shared_from_this());

    cerr << "miners: " << _server.frontend()->miners.find().dump() << endl;
}