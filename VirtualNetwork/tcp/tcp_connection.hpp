// Author: Oznabikhin Dmitry
// Email: gamexgroup@gmail.ru
//
// Copyright (c) UTS. All rights reserved.
#pragma once


//#include "../common-private.h"
#include "tcp_request_handler.hpp"
#include "tcp_reply.hpp"
#include "tcp_request_parser.hpp"
#include "tcp_request.hpp"

#include <array>
#include <memory>

#ifndef _MSC_VER
// save diagnostic state
#pragma GCC diagnostic push
// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wall"
//#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define BOOST_CONFIG_WARNING_DISABLE_HPP
#pragma GCC system_header
#endif

#include <boost/asio.hpp>

#ifdef WITH_SSL
#include <boost/asio/ssl.hpp>
#endif

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif


namespace vnet {

class connection_manager;

/**
 * @brief Thread session from client.
 */
class connection
    : public std::enable_shared_from_this<connection>
{
#ifdef WITH_SSL
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> connection_socket;
#else
	typedef boost::asio::ip::tcp::socket connection_socket;
#endif

public:
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    /// Construct a connection with the given socket.
	explicit connection(
#ifdef WITH_SSL // Because ssl stream don't supported move semantics.
		boost::asio::io_service& io_service_,
		boost::asio::ssl::context& context_,
#else
		connection_socket socket, 
#endif
		connection_manager& manager, request_handler& handler);

	connection_socket& socket()
    {
        return socket_;
    }

    /// Start the first asynchronous operation for the connection.
    void start();

    /// Stop all asynchronous operations associated with the connection.
    void stop();

private:
    /**
     * @brief Async read session.
     */
    void do_read();

    /**
     * @brief Push all async commands and write.
     */
    void do_write();

private:
    //! The socket used to communicate with the client.
	connection_socket socket_;

    /// The manager for this connection.
    connection_manager& connection_manager_;

    /// The handler used to process the incoming request.
    request_handler& request_handler_;

    /// Buffer for incoming data.
    //std::array<char, 8192> buffer_;
	std::vector<char> buffer_;

    /// The incoming request.
    request request_;

    /// The parser for the incoming request.
    request_parser request_parser_;

    /// The reply to be sent back to the client.
    reply reply_;
};

typedef std::shared_ptr<connection> connection_ptr;

} // namespace vnet.