#include "logging.h"
#include "server/listener.h"

boost::beast::string_view server::mime_type(string_view path) {
    using boost::beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if(pos == boost::beast::string_view::npos)
            return boost::beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

std::string server::path_cat(string_view base, string_view path) {
    if(base.empty()) {
        return path.to_string();
    }
    std::string result = base.to_string();
    char constexpr path_separator = '/';
    if(result.back() == path_separator) {
        result.resize(result.size() - 1);
    }
    result.append(path.data(), path.size());
    return result;
}

void server::fail(boost::system::error_code code, char const *what) {
    std::cerr << what << ": " << code.message() << std::endl;
}

server::Listener::Listener(
    io_context &ioc,
    context &ctx,
    tcp::endpoint endp,
    string &root
) : _ctx(ctx), _acceptor(ioc), _socket(ioc), _root(root) {
    error_code code;

    _acceptor.open(endp.protocol(), code);
    if(code) {
        fail(code, "open");
        return;
    }

    _acceptor.bind(endp, code);
    if(code) {
        fail(code, "bind");
        return;
    }

    // Start listening for connections
    _acceptor.listen(socket_base::max_listen_connections, code);
    if(code) {
        fail(code, "listen");
        return;
    }
}

void server::Listener::run() {
    if(!_acceptor.is_open()) {
        return;
    }
    do_accept();
}

void server::Listener::do_accept() {
    _acceptor.async_accept(
        _socket,
        bind(
            &Listener::on_accept,
            shared_from_this(),
            std::placeholders::_1
        )
    );
}

void server::Listener::on_accept(error_code code) {
    if(code) {
        fail(code, "accept");
    } else {
        make_shared<Detector>(move(_socket), _ctx, _root)->run();
    }
    do_accept();
}

server::Detector::Detector(tcp::socket socket, context &ctx, string &root) :
    _socket(move(socket)),
    _ctx(ctx),
    _strand(_socket.get_executor()),
    _root(root) {}

void server::Detector::run() {
    async_detect_ssl(
        _socket,
        _buffer,
        asio::bind_executor(
            _strand,
            bind(
                &Detector::on_detect,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        )
    );
}

void server::Detector::on_detect(error_code code, tribool result) {
    if(code) {
        return fail(code, "detect");
    }

    if(result) {
        make_shared<ssl_http_session>(move(_socket), _ctx, move(_buffer), _root)->run();
        return;
    }

    make_shared<plain_http_session>(move(_socket), move(_buffer), _root)->run();
}
