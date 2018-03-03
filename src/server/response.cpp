#include "server/response.h"

server::Response::response_type &&
server::Response::_base(
    http::request<http::string_body> &req,
    http::status status,
    const string &message
) {
    http::response<http::string_body> resp(status, req.version());
    resp.set(http::field::server, string_param(BOOST_BEAST_VERSION_STRING));
    resp.keep_alive(req.keep_alive());

    if (!message.empty()) {
        resp.set(http::field::content_type, string_param("text/html"));
        resp.body() = message;
        resp.prepare_payload();
    }

    return move(resp);
}

server::Response::response_type
server::Response::ok(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::ok,
        message
    ));
}

server::Response::response_type
server::Response::created(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::created,
        message
    ));
}

server::Response::response_type
server::Response::accepted(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::accepted,
        message
    ));
}

server::Response::response_type
server::Response::non_authoritative_information(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::non_authoritative_information,
        message
    ));
}

server::Response::response_type
server::Response::no_content(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::no_content,
        message
    ));
}

server::Response::response_type
server::Response::reset_content(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::reset_content,
        message
    ));
}

server::Response::response_type
server::Response::partial_content(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::partial_content,
        message
    ));
}

server::Response::response_type
server::Response::multi_status(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::multi_status,
        message
    ));
}

server::Response::response_type
server::Response::already_reported(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::already_reported,
        message
    ));
}

server::Response::response_type
server::Response::im_used(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::im_used,
        message
    ));
}

server::Response::response_type
server::Response::multiple_choices(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::multiple_choices,
        message
    ));
}

server::Response::response_type
server::Response::moved_permanently(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::moved_permanently,
        message
    ));
}

server::Response::response_type
server::Response::found(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::found,
        message
    ));
}

server::Response::response_type
server::Response::see_other(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::see_other,
        message
    ));
}

server::Response::response_type
server::Response::not_modified(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::not_modified,
        message
    ));
}

server::Response::response_type
server::Response::use_proxy(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::use_proxy,
        message
    ));
}

server::Response::response_type
server::Response::temporary_redirect(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::temporary_redirect,
        message
    ));
}

server::Response::response_type
server::Response::permanent_redirect(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::permanent_redirect,
        message
    ));
}

server::Response::response_type
server::Response::bad_request(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::bad_request,
        message
    ));
}

server::Response::response_type
server::Response::unauthorized(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::unauthorized,
        message
    ));
}

server::Response::response_type
server::Response::payment_required(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::payment_required,
        message
    ));
}

server::Response::response_type
server::Response::forbidden(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::forbidden,
        message
    ));
}

server::Response::response_type
server::Response::not_found(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::not_found,
        message
    ));
}

server::Response::response_type
server::Response::method_not_allowed(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::method_not_allowed,
        message
    ));
}

server::Response::response_type
server::Response::not_acceptable(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::not_acceptable,
        message
    ));
}

server::Response::response_type
server::Response::proxy_authentication_required(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::proxy_authentication_required,
        message
    ));
}

server::Response::response_type
server::Response::request_timeout(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::request_timeout,
        message
    ));
}

server::Response::response_type
server::Response::conflict(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::conflict,
        message
    ));
}

server::Response::response_type
server::Response::gone(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::gone,
        message
    ));
}

server::Response::response_type
server::Response::length_required(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::length_required,
        message
    ));
}

server::Response::response_type
server::Response::precondition_failed(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::precondition_failed,
        message
    ));
}

server::Response::response_type
server::Response::payload_too_large(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::payload_too_large,
        message
    ));
}

server::Response::response_type
server::Response::uri_too_long(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::uri_too_long,
        message
    ));
}

server::Response::response_type
server::Response::unsupported_media_type(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::unsupported_media_type,
        message
    ));
}

server::Response::response_type
server::Response::range_not_satisfiable(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::range_not_satisfiable,
        message
    ));
}

server::Response::response_type
server::Response::expectation_failed(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::expectation_failed,
        message
    ));
}

server::Response::response_type
server::Response::misdirected_request(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::misdirected_request,
        message
    ));
}

server::Response::response_type
server::Response::unprocessable_entity(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::unprocessable_entity,
        message
    ));
}

server::Response::response_type
server::Response::locked(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::locked,
        message
    ));
}

server::Response::response_type
server::Response::failed_dependency(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::failed_dependency,
        message
    ));
}

server::Response::response_type
server::Response::upgrade_required(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::upgrade_required,
        message
    ));
}

server::Response::response_type
server::Response::precondition_required(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::precondition_required,
        message
    ));
}

server::Response::response_type
server::Response::too_many_requests(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::too_many_requests,
        message
    ));
}

server::Response::response_type
server::Response::request_header_fields_too_large(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::request_header_fields_too_large,
        message
    ));
}

server::Response::response_type
server::Response::connection_closed_without_response(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::connection_closed_without_response,
        message
    ));
}

server::Response::response_type
server::Response::unavailable_for_legal_reasons(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::unavailable_for_legal_reasons,
        message
    ));
}

server::Response::response_type
server::Response::client_closed_request(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::client_closed_request,
        message
    ));
}

server::Response::response_type
server::Response::internal_server_error(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::internal_server_error,
        message
    ));
}

server::Response::response_type
server::Response::not_implemented(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::not_implemented,
        message
    ));
}

server::Response::response_type
server::Response::bad_gateway(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::bad_gateway,
        message
    ));
}

server::Response::response_type
server::Response::service_unavailable(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::service_unavailable,
        message
    ));
}

server::Response::response_type
server::Response::gateway_timeout(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::gateway_timeout,
        message
    ));
}

server::Response::response_type
server::Response::http_version_not_supported(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::http_version_not_supported,
        message
    ));
}

server::Response::response_type
server::Response::variant_also_negotiates(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::variant_also_negotiates,
        message
    ));
}

server::Response::response_type
server::Response::insufficient_storage(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::insufficient_storage,
        message
    ));
}

server::Response::response_type
server::Response::loop_detected(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::loop_detected,
        message
    ));
}

server::Response::response_type
server::Response::not_extended(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::not_extended,
        message
    ));
}

server::Response::response_type
server::Response::network_authentication_required(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::network_authentication_required,
        message
    ));
}

server::Response::response_type
server::Response::network_connect_timeout_error(
    http::request<http::string_body> &req,
    const string &message
) {
    return move(_base(
        req,
        http::status::network_connect_timeout_error,
        message
    ));
}