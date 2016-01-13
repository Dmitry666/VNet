// Author: Oznabikhin Dmitry
// Email: gamexgroup@gmail.ru
//
// Copyright (c) UTS. All rights reserved.
#pragma once

#include "TcpCommon.h"
#include "Delegate.h"


//#include "../common-private.h"
//#include "tcp_request_handler.hpp"
//#include "tcp_reply.hpp"
//#include "tcp_request_parser.hpp"
//#include "tcp_request.hpp"

#include <array>
#include <memory>
#include <mutex>
#include <deque>
#include <thread>

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

class TcpConnectionManager;

/**
 * @brief Thread session from client.
 */
class TcpConnection
	: public std::enable_shared_from_this<TcpConnection>
{
public:
#ifdef WITH_SSL
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> connection_socket;
#else
	typedef boost::asio::ip::tcp::socket connection_socket;
#endif

public:
	TcpConnection(const TcpConnection&) = delete;
	TcpConnection& operator=(const TcpConnection&) = delete;

    /// Construct a connection with the given socket.
	explicit TcpConnection(
#ifdef WITH_SSL // Because ssl stream don't supported move semantics.
		boost::asio::io_service& io_service_,
		boost::asio::ssl::context& context_,
#else
		connection_socket socket, 
#endif
		TcpConnectionManager& manager);

	connection_socket& socket()
    {
        return socket_;
    }


    /// Start the first asynchronous operation for the connection.
    void Start();

    /// Stop all asynchronous operations associated with the connection.
    void Stop();

	///
	void Send(const uint8* bytes, uint32 size);

public:
	Delegate<const uint8*, uint32> NewRequest;

private:
    /**
     * @brief Async read session.
     */
    void DoRead();

    /**
     * @brief Push all async commands and write.
     */
    void DoWrite();

private:
    //! The socket used to communicate with the client.
	connection_socket socket_;

    /// The manager for this connection.
	TcpConnectionManager& connection_manager_;

	//! The size of a fixed length header.
	enum { header_length = 4 };

	//! Holds an inbound header.
	char inbound_header_[header_length];

	//! Holds the inbound data.
	std::vector<uint8> inbound_data_;

	//! Holds the outbound data.
	std::vector<uint8> outbound_data_;

	std::deque<TcpCommand> _commands;				// Out data buffer from client.
	std::mutex _mutex;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

} // namespace vnet.