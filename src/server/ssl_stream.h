#ifndef SYNCAIDE_SERVER_SSL_STREAM_HPP
#define SYNCAIDE_SERVER_SSL_STREAM_HPP

#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

using namespace std;

namespace server {
    namespace ssl = boost::asio::ssl;
    using boost::system::error_code;
    using boost::asio::ip::tcp;
    using boost::asio::ssl::verify_mode;

    template<class NextLayer>
    class SslStream : public ssl::stream_base {
        using stream_type = ssl::stream<NextLayer>;

    private:
        unique_ptr<stream_type> _p;
        ssl::context* _ctx;

    public:
        using native_handle_type = typename stream_type::native_handle_type;
        using impl_struct = typename stream_type::impl_struct;
        using next_layer_type = typename stream_type::next_layer_type;
        using lowest_layer_type = typename stream_type::lowest_layer_type;
        using executor_type = typename stream_type::executor_type;

        SslStream(tcp::socket socket, ssl::context& ctx) :
            _p(new stream_type{socket.get_executor().context(), ctx}),
            _ctx(&ctx) {
            _p->next_layer() = move(socket);
        }

        SslStream(SslStream&& other) noexcept :
            _p(new stream_type(other.get_executor().context(), *other._ctx)),
            _ctx(other._ctx) {
            using std::swap;
            swap(_p, other._p);
        }

        SslStream& operator=(SslStream&& other) noexcept {
            std::unique_ptr<stream_type> p(new stream_type(
                other.get_executor().context(), 
                *other._ctx
            ));
            using std::swap;
            swap(_p, p);
            swap(_p, other._p);
            _ctx = other._ctx;
            return *this;
        }

        executor_type get_executor() noexcept {
            return _p->get_executor();
        }

        native_handle_type native_handle() {
            return _p->native_handle();
        }

        next_layer_type const&next_layer() const {
            return _p->next_layer();
        }

        next_layer_type&next_layer() {
            return _p->next_layer();
        }

        lowest_layer_type&lowest_layer() {
            return _p->lowest_layer();
        }

        lowest_layer_type const&lowest_layer() const {
            return _p->lowest_layer();
        }

        void set_verify_mode(boost::asio::ssl::verify_mode v) {
            _p->set_verify_mode(v);
        }

        error_code set_verify_mode(verify_mode v, error_code& code) {
            return _p->set_verify_mode(v, code);
        }

        void set_verify_depth(int depth) {
            _p->set_verify_depth(depth);
        }

        error_code set_verify_depth(int depth, error_code& code) {
            return _p->set_verify_depth(depth, code);
        }

        template<class VerifyCallback> 
        void set_verify_callback(VerifyCallback callback) {
            _p->set_verify_callback(callback);
        }

        template<class VerifyCallback>
        error_code set_verify_callback(
            VerifyCallback callback,
            error_code& code
        ) {
            return _p->set_verify_callback(callback, code);
        }

        void handshake(handshake_type type) {
            _p->handshake(type);
        }

        error_code handshake(handshake_type type, error_code& code) {
            return _p->handshake(type, code);
        }

        template<class ConstBufferSeq>
        void handshake(handshake_type type, ConstBufferSeq const& buffers) {
            _p->handshake(type, buffers);
        }

        template<class ConstBufferSeq>
        boost::system::error_code handshake(
            handshake_type type,
            ConstBufferSeq const& buffers,
            error_code& code
        ) {
            return _p->handshake(type, buffers, code);
        }

        template<class HandshakeHandler>
        BOOST_ASIO_INITFN_RESULT_TYPE(
            HandshakeHandler,
            void(boost::system::error_code)
        )
        async_handshake(handshake_type type,
            BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler
        ) {
            return _p->async_handshake(type, handler);
        }

        template<class ConstBufferSeq, class BufferedHandshakeHandler>
        BOOST_ASIO_INITFN_RESULT_TYPE(
            BufferedHandshakeHandler,
            void (boost::system::error_code, std::size_t)
        )
        async_handshake(
            handshake_type type,
            ConstBufferSeq const& buffers,
            BOOST_ASIO_MOVE_ARG(BufferedHandshakeHandler) handler
        ) {
            return _p->async_handshake(type, buffers, handler);
        }

        void shutdown() {
            _p->shutdown();
        }

        error_code shutdown(error_code& code) {
            return _p->shutdown(code);
        }

        template<class ShutdownHandler>
        BOOST_ASIO_INITFN_RESULT_TYPE(
            ShutdownHandler,
            void (boost::system::error_code)
        )
        async_shutdown(BOOST_ASIO_MOVE_ARG(ShutdownHandler) handler) {
            return _p->async_shutdown(handler);
        }

        template<class ConstBufferSeq>
        size_t write_some(ConstBufferSeq const& buffers) {
            return _p->write_some(buffers);
        }

        template<class ConstBufferSeq>
        size_t write_some(ConstBufferSeq const& buffers, error_code& code) {
            return _p->write_some(buffers, code);
        }

        template<class ConstBufferSeq, class WriteHandler>
        BOOST_ASIO_INITFN_RESULT_TYPE(
            WriteHandler,
            void (boost::system::error_code, std::size_t)
        )
        async_write_some(
            ConstBufferSeq const& buffers,
            BOOST_ASIO_MOVE_ARG(WriteHandler) handler
        ) {
            return _p->async_write_some(buffers, handler);
        }

        template<class MutableBufferSequence>
        size_t read_some(MutableBufferSequence const& buffers) {
            return _p->read_some(buffers);
        }

        template<class MutableBufferSequence>
        size_t read_some(
            MutableBufferSequence const& buffers,
            error_code& code
        ) {
            return _p->read_some(buffers, code);
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
            return _p->async_read_some(buffers, handler);
        }

        template<class SyncStream>
        friend void teardown(
            boost::beast::websocket::role_type,
            SslStream<SyncStream>& stream,
            error_code& code
        );

        template<class AsyncStream, class TeardownHandler>
        friend void async_teardown(
            boost::beast::websocket::role_type,
            SslStream<AsyncStream>& stream,
            TeardownHandler&& handler
        );
    };

    template<class SyncStream>
    inline void teardown(
        boost::beast::websocket::role_type role,
        SslStream<SyncStream>& stream,
        boost::system::error_code& code
    ) {
        using boost::beast::websocket::teardown;
        teardown(role, *stream._p, code);
    }

    template<class AsyncStream, class TeardownHandler>
    inline void async_teardown(
        boost::beast::websocket::role_type role,
        SslStream<AsyncStream>& stream,
        TeardownHandler&& handler
    ) {
        using boost::beast::websocket::async_teardown;
        async_teardown(role, *stream._p, forward<TeardownHandler>(handler));
    }
}

#endif