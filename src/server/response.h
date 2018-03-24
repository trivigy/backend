#ifndef SYNCAIDE_SERVER_RESPONSE_H
#define SYNCAIDE_SERVER_RESPONSE_H

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <utility>

using namespace std;

namespace server {
    namespace http = boost::beast::http;
    using boost::beast::string_param;
    using boost::beast::http::request;
    using boost::beast::http::response;
    using boost::beast::http::string_body;
    using boost::beast::http::field;

    class Response {
        using request_type = request<string_body>;
        using response_type = response<string_body>;

    private:
        static response_type _base(
            request_type &req,
            http::status status,
            const string &message = string()
        );

    public:
        // 2xx success
        static response_type ok(
            request_type &req,
            const string &message = string()
        );

        static response_type created(
            request_type &req,
            const string &message = string()
        );

        static response_type accepted(
            request_type &req,
            const string &message = string()
        );

        static response_type non_authoritative_information(
            request_type &req,
            const string &message = string()
        );

        static response_type no_content(
            request_type &req,
            const string &message = string()
        );

        static response_type reset_content(
            request_type &req,
            const string &message = string()
        );

        static response_type partial_content(
            request_type &req,
            const string &message = string()
        );

        static response_type multi_status(
            request_type &req,
            const string &message = string()
        );

        static response_type already_reported(
            request_type &req,
            const string &message = string()
        );

        static response_type im_used(
            request_type &req,
            const string &message = string()
        );

        // 3xx redirection
        static response_type multiple_choices(
            request_type &req,
            const string &message = string()
        );

        static response_type moved_permanently(
            request_type &req,
            const string &message = string()
        );

        static response_type found(
            request_type &req,
            const string &message = string()
        );

        static response_type see_other(
            request_type &req,
            const string &message = string()
        );

        static response_type not_modified(
            request_type &req,
            const string &message = string()
        );

        static response_type use_proxy(
            request_type &req,
            const string &message = string()
        );

        static response_type temporary_redirect(
            request_type &req,
            const string &message = string()
        );

        static response_type permanent_redirect(
            request_type &req,
            const string &message = string()
        );

        // 4xx client errors
        static response_type bad_request(
            request_type &req,
            const string &message = string()
        );

        static response_type unauthorized(
            request_type &req,
            const string &message = string()
        );

        static response_type payment_required(
            request_type &req,
            const string &message = string()
        );

        static response_type forbidden(
            request_type &req,
            const string &message = string()
        );

        static response_type not_found(
            request_type &req,
            const string &message = string()
        );

        static response_type method_not_allowed(
            request_type &req,
            const string &message = string()
        );

        static response_type not_acceptable(
            request_type &req,
            const string &message = string()
        );

        static response_type proxy_authentication_required(
            request_type &req,
            const string &message = string()
        );

        static response_type request_timeout(
            request_type &req,
            const string &message = string()
        );

        static response_type conflict(
            request_type &req,
            const string &message = string()
        );

        static response_type gone(
            request_type &req,
            const string &message = string()
        );

        static response_type length_required(
            request_type &req,
            const string &message = string()
        );

        static response_type precondition_failed(
            request_type &req,
            const string &message = string()
        );

        static response_type payload_too_large(
            request_type &req,
            const string &message = string()
        );

        static response_type uri_too_long(
            request_type &req,
            const string &message = string()
        );

        static response_type unsupported_media_type(
            request_type &req,
            const string &message = string()
        );

        static response_type range_not_satisfiable(
            request_type &req,
            const string &message = string()
        );

        static response_type expectation_failed(
            request_type &req,
            const string &message = string()
        );

        static response_type misdirected_request(
            request_type &req,
            const string &message = string()
        );

        static response_type unprocessable_entity(
            request_type &req,
            const string &message = string()
        );

        static response_type locked(
            request_type &req,
            const string &message = string()
        );

        static response_type failed_dependency(
            request_type &req,
            const string &message = string()
        );

        static response_type upgrade_required(
            request_type &req,
            const string &message = string()
        );

        static response_type precondition_required(
            request_type &req,
            const string &message = string()
        );

        static response_type too_many_requests(
            request_type &req,
            const string &message = string()
        );

        static response_type request_header_fields_too_large(
            request_type &req,
            const string &message = string()
        );

        static response_type connection_closed_without_response(
            request_type &req,
            const string &message = string()
        );

        static response_type unavailable_for_legal_reasons(
            request_type &req,
            const string &message = string()
        );

        static response_type client_closed_request(
            request_type &req,
            const string &message = string()
        );

        // 5xx server errors
        static response_type internal_server_error(
            request_type &req,
            const string &message = string()
        );

        static response_type not_implemented(
            request_type &req,
            const string &message = string()
        );

        static response_type bad_gateway(
            request_type &req,
            const string &message = string()
        );

        static response_type service_unavailable(
            request_type &req,
            const string &message = string()
        );

        static response_type gateway_timeout(
            request_type &req,
            const string &message = string()
        );

        static response_type http_version_not_supported(
            request_type &req,
            const string &message = string()
        );

        static response_type variant_also_negotiates(
            request_type &req,
            const string &message = string()
        );

        static response_type insufficient_storage(
            request_type &req,
            const string &message = string()
        );

        static response_type loop_detected(
            request_type &req,
            const string &message = string()
        );

        static response_type not_extended(
            request_type &req,
            const string &message = string()
        );

        static response_type network_authentication_required(
            request_type &req,
            const string &message = string()
        );


        static response_type network_connect_timeout_error(
            request_type &req,
            const string &message = string()
        );
    };
}

#endif //SYNCAIDE_SERVER_RESPONSE_H