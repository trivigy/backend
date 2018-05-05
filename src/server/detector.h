#ifndef SYNCAIDE_SERVER_DETECT_SSL_H
#define SYNCAIDE_SERVER_DETECT_SSL_H

#include "server/http.h"
#include "server/helper.h"
#include "server/router.h"

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/logic/tribool.hpp>

#define RESULT_TYPE(ct, sig) \
  typename ::boost::asio::async_result< \
    typename ::boost::asio::decay<ct>::type, sig>::return_type

#define HANDLER_TYPE(ct, sig) \
  typename ::boost::asio::async_result< \
    typename ::boost::asio::decay<ct>::type, sig>::completion_handler_type

#define HANDLER_SIG void(boost::beast::error_code, boost::tribool)

#define STREAM_EXEC_TYPE decltype(declval<Stream &>().get_executor())

using namespace std;

namespace server {
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    using boost::system::error_code;
    using boost::beast::flat_buffer;
    using boost::beast::read_size;
    using boost::asio::async_completion;
    using boost::asio::is_const_buffer_sequence;
    using boost::asio::get_associated_allocator;
    using boost::asio::associated_allocator_t;
    using boost::asio::associated_executor_t;
    using boost::asio::get_associated_executor;
    using boost::asio::buffer_size;
    using boost::asio::buffer_copy;
    using boost::beast::bind_handler;
    using boost::asio::buffer;
    using boost::asio::ip::tcp;
    using boost::asio::io_context;
    using boost::asio::ssl::context;
    using boost::asio::bind_executor;
    using boost::asio::strand;
    using boost::asio::post;
    using boost::indeterminate;
    using boost::tribool;

    template<class Stream, class Buffer, class Handler>
    class DetectSslOp {
    private:
        Stream &_stream;
        Buffer &_buffer;
        Handler _handler;
        tribool _result = false;
        int _step = 0;

    public:
        DetectSslOp(DetectSslOp const &) = default;

        template<class Deduce>
        DetectSslOp(Stream &stream, Buffer &buffer, Deduce &&handler) :
            _stream(stream),
            _buffer(buffer),
            _handler(forward<Deduce>(handler)) {}

        using exec_type = associated_executor_t<Handler, STREAM_EXEC_TYPE>;
        using alloc_type = associated_allocator_t<Handler>;

        alloc_type get_allocator() const noexcept {
            return get_associated_allocator(_handler);
        }

        exec_type get_executor() const noexcept {
            return get_associated_executor(_handler, _stream.get_executor());
        }

        friend bool asio_handler_is_continuation(DetectSslOp *op) {
            using boost::asio::asio_handler_is_continuation;
            return op->_step > 2
                   || asio_handler_is_continuation(addressof(op->_handler));
        }

        template<class ConstBufferSequence>
        tribool is_ssl_handshake(ConstBufferSequence const &buffers) {
            if (buffer_size(buffers) < 1) return indeterminate;

            unsigned char v;
            buffer_copy(buffer(&v, 1), buffers);

            if (v != 0x16) return false;
            if (buffer_size(buffers) < 4) return indeterminate;

            return true;
        }

        void operator()(error_code code, size_t bytes_transferred) {
            switch (_step) {
                case 0:
                    _result = is_ssl_handshake(_buffer.data());
                    if (!indeterminate(_result)) {
                        _step = 1;
                        return post(
                            _stream.get_executor(),
                            bind_handler(move(*this), code, 0)
                        );
                    }
                    _step = 2;

                do_read:
                    return _stream.async_read_some(
                        _buffer.prepare(read_size(_buffer, 1536)),
                        move(*this)
                    );

                case 1:
                    break;

                case 2:
                    _step = 3;
                    [[fallthrough]];

                case 3:
                    if (code) {
                        _result = false;
                        break;
                    }

                    _buffer.commit(bytes_transferred);
                    _result = is_ssl_handshake(_buffer.data());

                    if (!indeterminate(_result)) break;

                    goto do_read;

                default:
                    break;
            }

            _handler(code, _result);
        }
    };

    class Detector : public enable_shared_from_this<Detector> {
    private:
        tcp::socket _socket;
        context &_ctx;
        strand<io_context::executor_type> _strand;
        shared_ptr<Router> _router;
        flat_buffer _buffer;

    public:
        explicit Detector(
            tcp::socket socket,
            context &ctx,
            shared_ptr<Router> router
        );

        void run();

    private:
        void on_detect(error_code code, tribool secured);

        template<class Stream, class Buffer, class Token>
        RESULT_TYPE(Token, HANDLER_SIG)
        async_detect_ssl(Stream &stream, Buffer &buffer, Token &&token) {
            async_completion<Token, void(error_code, tribool)> init{token};

            DetectSslOp<Stream, Buffer, HANDLER_TYPE(Token, HANDLER_SIG)>{
                stream, buffer, init.completion_handler
            }(error_code{}, 0);

            return init.result.get();
        }
    };
}

#endif //SYNCAIDE_SERVER_DETECT_SSL_H
