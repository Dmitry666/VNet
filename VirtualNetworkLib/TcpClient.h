#pragma once

#include "TcpCommon.h"
#include "Delegate.h"


#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <mutex>
#include <deque>
#include <thread>


namespace vnet {

/**
 * @brief Network tcp client.
 */
class TcpClient
{
#ifdef WITH_SSL
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> Socket;
#else
	typedef boost::asio::ip::tcp::socket Socket;
#endif

public:
	TcpClient(bool autoreconnect = true);
	virtual ~TcpClient();

	void Connect(const std::string& address, uint32 port);
	void Disconnect();
	bool IsConnected();

	// Send message methods.
	void Ping(); // TODO. Special method. Architecture bug.
	void Send(const uint8* bytes, uint32 nbBytes);

public:
	Delegate<TcpClient*> Connected;
	Delegate<TcpClient*> ConnectError;
	Delegate<TcpClient*> Disconnected;
	Delegate<const uint8*, uint32> ReadyRead;

private:
#ifdef WITH_SSL
	bool VerifyCertificate(bool preverified,boost::asio::ssl::verify_context& ctx);
#endif

	void WaitReconnect();
	bool Reconnect();
	void DoRead();
	void DoWrite();

	void BeginTimeout();

private:
#ifdef WITH_SSL
	boost::asio::ssl::context ctx_;
#endif

	boost::asio::io_service io_service;
	/// Server thread.
	std::thread thread_;

	// The socket used to communicate with the client.
#ifdef WITH_SSL
	//std::auto_ptr<Socket> socket_;
	Socket socket_;
#else
	Socket socket_;
#endif

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

	std::string _address;
	uint32 _port;
	bool _connected;

	boost::asio::deadline_timer waitTimer_;
	boost::asio::deadline_timer reconnectTimer_;
	boost::asio::deadline_timer timerTimeout_;

	bool _autoreconnect;
	bool _errorconnect;
	uint32 _reconnectCount;
};

} // End vnet.