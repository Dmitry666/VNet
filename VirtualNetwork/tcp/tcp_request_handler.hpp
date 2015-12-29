//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCP_REQUEST_HANDLER_HPP
#define TCP_REQUEST_HANDLER_HPP

#include <string>

namespace vnet {

struct reply;
struct request;

class SessionManager;

/// The common handler for all incoming requests.
class request_handler
{
public:
    request_handler(const request_handler&) = delete;
    request_handler& operator=(const request_handler&) = delete;

    /// Construct with a directory containing files to be served.
	explicit request_handler(SessionManager& manager);

    /// Handle a request and produce a reply.
	void handle_new_connection();
	void handle_remove_connection(const std::string& key);

    void handle_request(const request& req, reply& rep);

private:
	void handle_login(const request& req, reply& rep);
	void handle_logout(const request& req, reply& rep);
	void handle_new_address(const request& req, reply& rep);
	void handle_read_clients(const request& req, reply& rep);
	void handle_ping(const request& req, reply& rep);

	void handle_connect_client(const request& req, reply& rep);
	void handle_disconnect_client(const request& req, reply& rep);
	void handle_send_command(const request& req, reply& rep);
	void handle_read_command(const request& req, reply& rep);
	void handle_read_commands(const request& req, reply& rep);

private:
	SessionManager& manager_;
};

} // namespace vnet.

#endif // HTTP_REQUEST_HANDLER_HPP
