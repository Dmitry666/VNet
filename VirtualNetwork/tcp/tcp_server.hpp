// Author: Oznabikhin Dmitry
// Email: gamexgroup@gmail.ru
//
// Copyright (c) GameX. All rights reserved.
#pragma once

#include "tcp_connection.hpp"
#include "tcp_connection_manager.hpp"
#include "tcp_request_handler.hpp"

#include <thread>

namespace vnet {

/**
    Main net server class.
*/
class tcp_server //: public base_server
{
#ifdef WITH_SSL
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_type;
#else
	typedef boost::asio::ip::tcp::socket socket_type;
#endif
public:
    tcp_server(const tcp_server&) = delete;
    tcp_server& operator=(const tcp_server&) = delete;

    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
	explicit tcp_server(const std::string& address, const std::string& port, request_handler& request_handler);


    /// Run the server's io_service loop.
	void start();

	void stop();

private:
    /// Perform an asynchronous accept operation.
    void do_accept();

    /// Wait for a request to stop the server.
    void do_await_stop();

private:
    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service io_service_;

#ifdef WITH_SSL
    boost::asio::ssl::context context_;
#endif

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// The connection manager which owns all live connections.
    connection_manager connection_manager_;

#ifndef WITH_SSL
    /// The next socket to be accepted.
	socket_type socket_;
#endif

    /// The handler for all incoming requests.
    request_handler& request_handler_;


	//
	std::thread thread_;
};

} // namespace vnet.