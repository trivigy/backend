#ifndef SYNCAIDE_SERVER_RESPONSE_H
#define SYNCAIDE_SERVER_RESPONSE_H

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <utility>

using namespace std;

namespace server {
    namespace http = boost::beast::http;
    using boost::beast::string_param;

    class Response {
        using request_type = http::request<http::string_body>;
        using response_type = http::response<http::string_body>;

    private:
        static response_type &&_base(
            http::request<http::string_body> &req,
            http::status status,
            const string &message = string()
        );

    public:
        // 2xx success
        static response_type ok(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type created(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type accepted(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type non_authoritative_information(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type no_content(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type reset_content(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type partial_content(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type multi_status(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type already_reported(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type im_used(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        // 3xx redirection
        static response_type multiple_choices(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type moved_permanently(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type found(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type see_other(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type not_modified(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type use_proxy(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type temporary_redirect(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type permanent_redirect(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        // 4xx client errors
        static response_type bad_request(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type unauthorized(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type payment_required(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type forbidden(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type not_found(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type method_not_allowed(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type not_acceptable(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type proxy_authentication_required(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type request_timeout(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type conflict(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type gone(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type length_required(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type precondition_failed(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type payload_too_large(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type uri_too_long(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type unsupported_media_type(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type range_not_satisfiable(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type expectation_failed(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type misdirected_request(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type unprocessable_entity(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type locked(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type failed_dependency(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type upgrade_required(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type precondition_required(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type too_many_requests(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type request_header_fields_too_large(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type connection_closed_without_response(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type unavailable_for_legal_reasons(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type client_closed_request(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        // 5xx server errors
        static response_type internal_server_error(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type not_implemented(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type bad_gateway(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type service_unavailable(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type gateway_timeout(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type http_version_not_supported(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type variant_also_negotiates(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type insufficient_storage(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type loop_detected(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type not_extended(
            http::request<http::string_body> &req,
            const string &message = string()
        );

        static response_type network_authentication_required(
            http::request<http::string_body> &req,
            const string &message = string()
        );


        static response_type network_connect_timeout_error(
            http::request<http::string_body> &req,
            const string &message = string()
        );
    };
}

#endif //SYNCAIDE_SERVER_RESPONSE_H