#include "server/response.h"

server::Response::response_type
server::Response::_base(
    request_type &req,
    http::status status,
    const string &message
) {
    http::response<http::string_body> resp(status, req.version());
    resp.set(field::server, string_param(BOOST_BEAST_VERSION_STRING));
    resp.keep_alive(req.keep_alive());

    if (!message.empty()) {
        resp.set(field::content_type, string_param("text/html"));
        resp.body() = message;
        resp.prepare_payload();
    }

    return resp;
}

server::Response::response_type
server::Response::ok(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::ok,
        message
    );
}

server::Response::response_type
server::Response::created(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::created,
        message
    );
}

server::Response::response_type
server::Response::accepted(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::accepted,
        message
    );
}

server::Response::response_type
server::Response::non_authoritative_information(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::non_authoritative_information,
        message
    );
}

server::Response::response_type
server::Response::no_content(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::no_content,
        message
    );
}

server::Response::response_type
server::Response::reset_content(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::reset_content,
        message
    );
}

server::Response::response_type
server::Response::partial_content(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::partial_content,
        message
    );
}

server::Response::response_type
server::Response::multi_status(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::multi_status,
        message
    );
}

server::Response::response_type
server::Response::already_reported(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::already_reported,
        message
    );
}

server::Response::response_type
server::Response::im_used(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::im_used,
        message
    );
}

server::Response::response_type
server::Response::multiple_choices(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::multiple_choices,
        message
    );
}

server::Response::response_type
server::Response::moved_permanently(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::moved_permanently,
        message
    );
}

server::Response::response_type
server::Response::found(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::found,
        message
    );
}

server::Response::response_type
server::Response::see_other(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::see_other,
        message
    );
}

server::Response::response_type
server::Response::not_modified(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::not_modified,
        message
    );
}

server::Response::response_type
server::Response::use_proxy(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::use_proxy,
        message
    );
}

server::Response::response_type
server::Response::temporary_redirect(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::temporary_redirect,
        message
    );
}

server::Response::response_type
server::Response::permanent_redirect(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::permanent_redirect,
        message
    );
}

server::Response::response_type
server::Response::bad_request(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::bad_request,
        message
    );
}

server::Response::response_type
server::Response::unauthorized(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::unauthorized,
        message
    );
}

server::Response::response_type
server::Response::payment_required(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::payment_required,
        message
    );
}

server::Response::response_type
server::Response::forbidden(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::forbidden,
        message
    );
}

server::Response::response_type
server::Response::not_found(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::not_found,
        message
    );
}

server::Response::response_type
server::Response::method_not_allowed(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::method_not_allowed,
        message
    );
}

server::Response::response_type
server::Response::not_acceptable(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::not_acceptable,
        message
    );
}

server::Response::response_type
server::Response::proxy_authentication_required(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::proxy_authentication_required,
        message
    );
}

server::Response::response_type
server::Response::request_timeout(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::request_timeout,
        message
    );
}

server::Response::response_type
server::Response::conflict(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::conflict,
        message
    );
}

server::Response::response_type
server::Response::gone(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::gone,
        message
    );
}

server::Response::response_type
server::Response::length_required(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::length_required,
        message
    );
}

server::Response::response_type
server::Response::precondition_failed(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::precondition_failed,
        message
    );
}

server::Response::response_type
server::Response::payload_too_large(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::payload_too_large,
        message
    );
}

server::Response::response_type
server::Response::uri_too_long(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::uri_too_long,
        message
    );
}

server::Response::response_type
server::Response::unsupported_media_type(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::unsupported_media_type,
        message
    );
}

server::Response::response_type
server::Response::range_not_satisfiable(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::range_not_satisfiable,
        message
    );
}

server::Response::response_type
server::Response::expectation_failed(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::expectation_failed,
        message
    );
}

server::Response::response_type
server::Response::misdirected_request(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::misdirected_request,
        message
    );
}

server::Response::response_type
server::Response::unprocessable_entity(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::unprocessable_entity,
        message
    );
}

server::Response::response_type
server::Response::locked(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::locked,
        message
    );
}

server::Response::response_type
server::Response::failed_dependency(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::failed_dependency,
        message
    );
}

server::Response::response_type
server::Response::upgrade_required(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::upgrade_required,
        message
    );
}

server::Response::response_type
server::Response::precondition_required(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::precondition_required,
        message
    );
}

server::Response::response_type
server::Response::too_many_requests(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::too_many_requests,
        message
    );
}

server::Response::response_type
server::Response::request_header_fields_too_large(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::request_header_fields_too_large,
        message
    );
}

server::Response::response_type
server::Response::connection_closed_without_response(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::connection_closed_without_response,
        message
    );
}

server::Response::response_type
server::Response::unavailable_for_legal_reasons(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::unavailable_for_legal_reasons,
        message
    );
}

server::Response::response_type
server::Response::client_closed_request(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::client_closed_request,
        message
    );
}

server::Response::response_type
server::Response::internal_server_error(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::internal_server_error,
        message
    );
}

server::Response::response_type
server::Response::not_implemented(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::not_implemented,
        message
    );
}

server::Response::response_type
server::Response::bad_gateway(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::bad_gateway,
        message
    );
}

server::Response::response_type
server::Response::service_unavailable(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::service_unavailable,
        message
    );
}

server::Response::response_type
server::Response::gateway_timeout(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::gateway_timeout,
        message
    );
}

server::Response::response_type
server::Response::http_version_not_supported(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::http_version_not_supported,
        message
    );
}

server::Response::response_type
server::Response::variant_also_negotiates(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::variant_also_negotiates,
        message
    );
}

server::Response::response_type
server::Response::insufficient_storage(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::insufficient_storage,
        message
    );
}

server::Response::response_type
server::Response::loop_detected(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::loop_detected,
        message
    );
}

server::Response::response_type
server::Response::not_extended(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::not_extended,
        message
    );
}

server::Response::response_type
server::Response::network_authentication_required(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::network_authentication_required,
        message
    );
}

server::Response::response_type
server::Response::network_connect_timeout_error(
    request_type &req,
    const string &message
) {
    return _base(
        req,
        http::status::network_connect_timeout_error,
        message
    );
}