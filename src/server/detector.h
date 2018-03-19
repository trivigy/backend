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

using namespace std;

template<class ConstBufferSequence>
boost::tribool is_ssl_handshake(ConstBufferSequence const &buffers) {
    static_assert(
        boost::asio::is_const_buffer_sequence<ConstBufferSequence>::value,
        "ConstBufferSequence requirements not met"
    );

    if (boost::asio::buffer_size(buffers) < 1) {
        return boost::indeterminate;
    }

    unsigned char v;
    boost::asio::buffer_copy(boost::asio::buffer(&v, 1), buffers);

    if (v != 0x16) {
        return false;
    }

    if (boost::asio::buffer_size(buffers) < 4) {
        return boost::indeterminate;
    }

    return true;
}

template<class SyncReadStream, class DynamicBuffer>
boost::tribool detect_ssl(
    SyncReadStream &stream,
    DynamicBuffer &buffer,
    boost::beast::error_code &code
) {
    namespace beast = boost::beast;

    static_assert(
        beast::is_sync_read_stream<SyncReadStream>::value,
        "SyncReadStream requirements not met"
    );

    static_assert(
        boost::asio::is_dynamic_buffer<DynamicBuffer>::value,
        "DynamicBuffer requirements not met"
    );

    while (true) {
        auto const result = is_ssl_handshake(buffer.data());

        if (!boost::indeterminate(result)) {
            code.assign(0, code.category());
            return result;
        }

        BOOST_ASSERT(buffer.size() < 4);
        auto const mutable_buffer = buffer.prepare(
            beast::read_size(buffer, 1536)
        );

        size_t const bytes_transferred = stream.read_some(mutable_buffer, code);

        if (code) {
            break;
        }

        buffer.commit(bytes_transferred);
    }

    return false;
}

template<class AsyncReadStream, class DynamicBuffer, class Handler>
class detect_ssl_op;

template<class AsyncReadStream, class DynamicBuffer, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(
    CompletionToken,
    void(boost::beast::error_code, boost::tribool)
)
async_detect_ssl(
    AsyncReadStream &stream,
    DynamicBuffer &buffer,
    CompletionToken &&token
) {
    namespace beast = boost::beast;

    static_assert(
        beast::is_async_read_stream<AsyncReadStream>::value,
        "SyncReadStream requirements not met"
    );

    static_assert(
        boost::asio::is_dynamic_buffer<DynamicBuffer>::value,
        "DynamicBuffer requirements not met"
    );

    boost::asio::async_completion<
        CompletionToken,
        void(beast::error_code, boost::tribool)
    > init{token};

    detect_ssl_op<
        AsyncReadStream,
        DynamicBuffer,
        BOOST_ASIO_HANDLER_TYPE(
            CompletionToken,
            void(beast::error_code, boost::tribool)
        )
    > {stream, buffer, init.completion_handler}(beast::error_code{}, 0);

    return init.result.get();
}

template<class AsyncReadStream, class DynamicBuffer, class Handler>
class detect_ssl_op {
    int step_ = 0;

    AsyncReadStream &stream_;
    DynamicBuffer &buffer_;
    Handler handler_;
    boost::tribool result_ = false;

public:
    detect_ssl_op(detect_ssl_op const &) = default;

    template<class DeducedHandler>
    detect_ssl_op(
        AsyncReadStream &stream,
        DynamicBuffer &buffer,
        DeducedHandler &&handler
    ) : stream_(stream),
        buffer_(buffer),
        handler_(forward<DeducedHandler>(handler)) {}

    using allocator_type = boost::asio::associated_allocator_t<Handler>;

    allocator_type get_allocator() const noexcept {
        return boost::asio::get_associated_allocator(handler_);
    }

    using executor_type = boost::asio::associated_executor_t<
        Handler,
        decltype(declval<AsyncReadStream &>().get_executor())
    >;

    executor_type get_executor() const noexcept {
        return boost::asio::get_associated_executor(
            handler_,
            stream_.get_executor()
        );
    }

    friend bool asio_handler_is_continuation(detect_ssl_op *op) {
        using boost::asio::asio_handler_is_continuation;

        return op->step_ > 2
               || asio_handler_is_continuation(addressof(op->handler_));
    }

    void operator()(boost::beast::error_code code, size_t bytes_transferred);
};

template<class AsyncStream, class DynamicBuffer, class Handler>
void detect_ssl_op<AsyncStream, DynamicBuffer, Handler>::operator()(
    boost::beast::error_code code,
    size_t bytes_transferred
) {
    namespace beast = boost::beast;

    switch (step_) {
        case 0:
            result_ = is_ssl_handshake(buffer_.data());

            if (!boost::indeterminate(result_)) {
                step_ = 1;
                return boost::asio::post(
                    stream_.get_executor(),
                    beast::bind_handler(move(*this), code, 0));
            }

            BOOST_ASSERT(buffer_.size() < 4);

            step_ = 2;

        do_read:
            return stream_.async_read_some(
                buffer_.prepare(beast::read_size(buffer_, 1536)),
                move(*this)
            );

        case 1:
            break;

        case 2:
            step_ = 3;
                BOOST_FALLTHROUGH;

        case 3:
            if (code) {
                result_ = false;
                break;
            }

            buffer_.commit(bytes_transferred);
            result_ = is_ssl_handshake(buffer_.data());

            if (!boost::indeterminate(result_)) {
                break;
            }

            goto do_read;
    }

    handler_(code, result_);
}

namespace server {
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    using boost::system::error_code;
    using boost::beast::flat_buffer;
    using boost::asio::ip::tcp;
    using boost::asio::io_context;
    using boost::asio::ssl::context;
    using boost::asio::bind_executor;
    using boost::asio::strand;
    using boost::tribool;

    class Detector : public enable_shared_from_this<Detector> {
    public:
        explicit Detector(
            tcp::socket socket,
            context &ctx,
            Router &router
        );

        void run();

        void on_detect(error_code code, tribool secured);

    private:
        tcp::socket _socket;
        context &_ctx;
        strand<io_context::executor_type> _strand;
        Router &_router;
        flat_buffer _buffer;
    };
}

#endif //SYNCAIDE_SERVER_DETECT_SSL_H
