#ifndef SYNCAIDE_SERVER_SSL_STREAM_HPP
#define SYNCAIDE_SERVER_SSL_STREAM_HPP

#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

template<class NextLayer>
class ssl_stream : public boost::asio::ssl::stream_base {
    static_assert(std::is_same<NextLayer, boost::asio::ip::tcp::socket>::value, "NextLayer requirements not met");
    using stream_type = boost::asio::ssl::stream<NextLayer>;

private:
    std::unique_ptr<stream_type> p_;
    boost::asio::ssl::context* ctx_;

public:
    using native_handle_type = typename stream_type::native_handle_type;
    using impl_struct = typename stream_type::impl_struct;
    using next_layer_type = typename stream_type::next_layer_type;
    using lowest_layer_type = typename stream_type::lowest_layer_type;
    using executor_type = typename stream_type::executor_type;

    ssl_stream(
        boost::asio::ip::tcp::socket socket,
        boost::asio::ssl::context& ctx
    ) : p_(new stream_type{socket.get_executor().context(), ctx}),
        ctx_(&ctx) {
        p_->next_layer() = std::move(socket);
    }

    ssl_stream(ssl_stream&& other) :
        p_(new stream_type(other.get_executor().context(), *other.ctx_)),
        ctx_(other.ctx_) {
        using std::swap;
        swap(p_, other.p_);
    }

    ssl_stream& operator=(ssl_stream&& other) {
        std::unique_ptr<stream_type> p(new stream_type{other.get_executor().context(), other.ctx_});
        using std::swap;
        swap(p_, p);
        swap(p_, other.p_);
        ctx_ = other.ctx_;
        return *this;
    }

    executor_type get_executor() noexcept {
        return p_->get_executor();
    }

    native_handle_type native_handle() {
        return p_->native_handle();
    }

    next_layer_type const&next_layer() const {
        return p_->next_layer();
    }

    next_layer_type&next_layer() {
        return p_->next_layer();
    }

    lowest_layer_type&lowest_layer() {
        return p_->lowest_layer();
    }

    lowest_layer_type const&lowest_layer() const {
        return p_->lowest_layer();
    }

    void set_verify_mode(boost::asio::ssl::verify_mode v) {
        p_->set_verify_mode(v);
    }

    boost::system::error_code set_verify_mode(
        boost::asio::ssl::verify_mode v,
        boost::system::error_code& ec
    ) {
        return p_->set_verify_mode(v, ec);
    }

    void set_verify_depth(int depth) {
        p_->set_verify_depth(depth);
    }

    boost::system::error_code set_verify_depth(
        int depth,
        boost::system::error_code& ec
    ) {
        return p_->set_verify_depth(depth, ec);
    }

    template<class VerifyCallback> void set_verify_callback(
        VerifyCallback callback
    ) {
        p_->set_verify_callback(callback);
    }

    template<class VerifyCallback>
    boost::system::error_code set_verify_callback(
        VerifyCallback callback,
        boost::system::error_code& ec
    ) {
        return p_->set_verify_callback(callback, ec);
    }

    void handshake(handshake_type type) {
        p_->handshake(type);
    }

    boost::system::error_code handshake(
        handshake_type type,
        boost::system::error_code& ec
    ) {
        return p_->handshake(type, ec);
    }

    template<class ConstBufferSequence>
    void handshake(
        handshake_type type,
        ConstBufferSequence const& buffers
    ) {
        p_->handshake(type, buffers);
    }

    template<class ConstBufferSequence>
    boost::system::error_code handshake(
        handshake_type type,
        ConstBufferSequence const& buffers,
        boost::system::error_code& ec
    ) {
        return p_->handshake(type, buffers, ec);
    }

    template<class HandshakeHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        HandshakeHandler,
        void(boost::system::error_code)
    )
    async_handshake(handshake_type type,
        BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler
    ) {
        return p_->async_handshake(
            type,
            BOOST_ASIO_MOVE_CAST(HandshakeHandler)(handler)
        );
    }

    template<class ConstBufferSequence, class BufferedHandshakeHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        BufferedHandshakeHandler,
        void (boost::system::error_code, std::size_t)
    )
    async_handshake(
        handshake_type type,
        ConstBufferSequence const& buffers,
        BOOST_ASIO_MOVE_ARG(BufferedHandshakeHandler) handler
    ) {
        return p_->async_handshake(
            type,
            buffers,
            BOOST_ASIO_MOVE_CAST(BufferedHandshakeHandler)(handler)
        );
    }

    void shutdown() {
        p_->shutdown();
    }

    boost::system::error_code shutdown(boost::system::error_code& ec) {
        return p_->shutdown(ec);
    }

    template<class ShutdownHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        ShutdownHandler,
        void (boost::system::error_code)
    )
    async_shutdown(BOOST_ASIO_MOVE_ARG(ShutdownHandler) handler) {
        return p_->async_shutdown(
            BOOST_ASIO_MOVE_CAST(ShutdownHandler)(handler)
        );
    }

    template<class ConstBufferSequence>
    std::size_t write_some(ConstBufferSequence const& buffers) {
        return p_->write_some(buffers);
    }

    template<class ConstBufferSequence>
    std::size_t write_some(
        ConstBufferSequence const& buffers,
        boost::system::error_code& ec
    ) {
        return p_->write_some(buffers, ec);
    }

    template<class ConstBufferSequence, class WriteHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        WriteHandler,
        void (boost::system::error_code, std::size_t)
    )
    async_write_some(
        ConstBufferSequence const& buffers,
        BOOST_ASIO_MOVE_ARG(WriteHandler) handler
    ) {
        return p_->async_write_some(buffers,
            BOOST_ASIO_MOVE_CAST(WriteHandler)(handler)
        );
    }

    template<class MutableBufferSequence>
    std::size_t read_some(MutableBufferSequence const& buffers) {
        return p_->read_some(buffers);
    }

    template<class MutableBufferSequence>
    std::size_t read_some(
        MutableBufferSequence const& buffers,
        boost::system::error_code& ec
    ) {
        return p_->read_some(buffers, ec);
    }

    template<class MutableBufferSequence, class ReadHandler>
    BOOST_ASIO_INITFN_RESULT_TYPE(
        ReadHandler,
        void(boost::system::error_code, std::size_t)
    )
    async_read_some(
        MutableBufferSequence const& buffers,
        BOOST_ASIO_MOVE_ARG(ReadHandler) handler
    ) {
        return p_->async_read_some(buffers,
            BOOST_ASIO_MOVE_CAST(ReadHandler)(handler)
        );
    }

    template<class SyncStream>
    friend void teardown(
        boost::beast::websocket::role_type,
        ssl_stream<SyncStream>& stream,
        boost::system::error_code& ec
    );

    template<class AsyncStream, class TeardownHandler>
    friend void async_teardown(
        boost::beast::websocket::role_type,
        ssl_stream<AsyncStream>& stream,
        TeardownHandler&& handler
    );
};

template<class SyncStream>
inline void teardown(
    boost::beast::websocket::role_type role,
    ssl_stream<SyncStream>& stream,
    boost::system::error_code& ec
) {
    using boost::beast::websocket::teardown;
    teardown(role, *stream.p_, ec);
}

template<class AsyncStream, class TeardownHandler>
inline void async_teardown(
    boost::beast::websocket::role_type role,
    ssl_stream<AsyncStream>& stream,
    TeardownHandler&& handler
) {
    using boost::beast::websocket::async_teardown;
    async_teardown(role, *stream.p_, std::forward<TeardownHandler>(handler));
}

#endif