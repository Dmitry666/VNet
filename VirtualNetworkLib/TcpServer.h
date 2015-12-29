// Author: Oznabikhin Dmitry
// Email: gamexgroup@gmail.ru
//
// Copyright (c) GameX. All rights reserved.
#pragma once

#include "Common.h"

#include "TcpConnection.h"
#include "TcpConnectionManager.h"

#include "Delegate.h"

#include <thread>

namespace vnet {

/**
 * @brief Tcp network server class.
 */
class TcpServer
{
#ifdef WITH_SSL
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_type;
#else
	typedef boost::asio::ip::tcp::socket socket_type;
#endif

public:
	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;

    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
	explicit TcpServer(const std::string& address, const std::string& port);
	virtual ~TcpServer();

    ///
	void Start();

	///
	void Stop();

#if 0
	TcpConnection* Accept()
	{
		acceptor_.accept(socket_);

		//connection_manager_.start(std::make_shared<connection>(
		//	std::move(socket_), connection_manager_, request_handler_));

		TcpConnection* tcpConnection = nullptr;
		return tcpConnection;
	}

	template<typename TFunc>
	void AcceptAsync()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
			{
				// Check whether the server was stopped by a signal before this
				// completion handler had a chance to run.
				if (!acceptor_.is_open())
				{
					return;
				}

				if (!ec)
				{
					connection_manager_.start(std::make_shared<connection>(
						std::move(socket_), connection_manager_, request_handler_));
				}

				do_accept();
			}
		);
	}
#endif

public:
	Delegate<TcpConnection*> NewConnection;
	Delegate<TcpConnection*> RemoveConnection;

private:
    /// Perform an asynchronous accept operation.
    void DoAccept();

    /// Wait for a request to stop the server.
    void DoAwaitStop();

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
	TcpConnectionManager connection_manager_;

#ifndef WITH_SSL
    /// The next socket to be accepted.
	socket_type socket_;
#endif

	/// Server thread.
	std::thread thread_;
};

} // namespace vnet.