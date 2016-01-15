#include "TcpClient.h"

#include <functional>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/diagnostic_information.hpp>

using boost::asio::ip::tcp;
//using namespace boost::interprocess;

#define     NET_CMD_LOGIN_SANDBOX           8
#define		NET_CMD_UPDATE_EMPTY			9

#define TCP_SOCKET_PERIOD 300

//#ifdef _DEBUG
//#define TIMEOUT_TIME 3600
//#else
#define TIMEOUT_TIME 5
//#endif

namespace vnet {

TcpClient::TcpClient(bool autoreconnect)
#ifdef WITH_SSL
	: ctx_(boost::asio::ssl::context::sslv23)
	, socket_(io_service, ctx_)
#else
	: socket_(io_service)
#endif
	, _connected(false)

	, waitTimer_(io_service)
	, reconnectTimer_(io_service)
	, timerTimeout_(io_service)

	, _autoreconnect(autoreconnect)
	, _errorconnect(false)
	, _reconnectCount(0)
{
#ifdef WITH_SSL
	//ctx_.set_verify_mode(boost::asio::ssl::context::verify_peer);
	try
	{
		ctx_.set_verify_mode(boost::asio::ssl::context::verify_none);
		ctx_.load_verify_file("server.crt");
	}
	catch (std::exception e)
	{
		//throw std::string("bad file sertificate ") + e.what();

		std::cout << "Unexpected exception caught in " <<
			BOOST_CURRENT_FUNCTION << std::endl <<
			boost::current_exception_diagnostic_information();
		return;
	}
#endif

#ifdef WITH_SSL
	//socket_.reset(new boost::asio::ssl::stream<tcp::socket>(io_service, ctx_));

	socket_.set_verify_mode(boost::asio::ssl::verify_peer);
	socket_.set_verify_callback(
		std::bind(&TcpClient::VerifyCertificate, this, std::placeholders::_1, std::placeholders::_2));
#endif
}

TcpClient::~TcpClient()
{
	io_service.stop();
	if(thread_.joinable())
		thread_.join();
}

void TcpClient::Connect(const std::string& address, uint32 port)
{
	_address = address;
	_port = port;

	_errorconnect = false;
	_reconnectCount = 0;


	if (!Reconnect())
	{
		ConnectError.Invoke(this);
		return;
	}

	thread_ = std::thread([this](){

		// The io_service::run() call will block until all asynchronous operations
		// have finished. While the server is running, there is always at least one
		// asynchronous operation outstanding: the asynchronous accept call waiting
		// for new incoming connections.
		io_service.run();
	});
}

void TcpClient::Disconnect()
{
	_connected = false;

	try
	{
	#ifdef WITH_SSL
		if(socket_.lowest_layer().is_open())
			socket_.lowest_layer().close();
#else
		if (socket_.is_open())
			socket_.close();
#endif
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	io_service.stop();

	if (thread_.joinable())
		thread_.join();

	Disconnected.Invoke(this);
}

bool TcpClient::IsConnected()
{
	//if (_connected)
	//	return socket_.is_open();

	return _connected;
}

void TcpClient::Ping()
{
	Send(nullptr, 0);
}

void TcpClient::Send(const uint8* bytes, uint32 nbBytes)
{
	std::lock_guard<std::mutex> _lock(_mutex);
	_commands.push_back(TcpCommand(bytes, nbBytes));
}

#ifdef WITH_SSL
bool TcpClient::VerifyCertificate(bool preverified, boost::asio::ssl::verify_context& ctx)
{
	// The verify callback can be used to check whether the certificate that is
	// being presented is valid for the peer. For example, RFC 2818 describes
	// the steps involved in doing this for HTTPS. Consult the OpenSSL
	// documentation for more details. Note that the callback is called once
	// for each certificate in the certificate chain, starting from the root
	// certificate authority.

	// In this example we will simply print the certificate's subject name.
	char subject_name[256];
	X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
	X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
	std::cout << "Verifying " << subject_name << "\n";

	return preverified;
}
#endif

void TcpClient::WaitReconnect()
{
	reconnectTimer_.cancel();

	if (_errorconnect)
	{
		return;
	}

	if (!_autoreconnect || _reconnectCount > 5)
	{
		_errorconnect = true;
		ConnectError.Invoke(this);
		return;
	}



	reconnectTimer_.expires_from_now(boost::posix_time::seconds(5));
	
	++_reconnectCount;
	reconnectTimer_.async_wait([this](const boost::system::error_code& ec){
		if (ec != boost::asio::error::operation_aborted)
		{
			Reconnect();
		}
	});
}

bool TcpClient::Reconnect()
{
	// Resolve the host name into an IP address.

	try
	{
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(_address, boost::lexical_cast<std::string>(_port));
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		BeginTimeout();

		// Start an asynchronous connect operation.
#ifdef WITH_SSL
		boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
			[this](const boost::system::error_code& ec, const tcp::resolver::iterator& it){
				if (!ec)
				{
					_connected = true;
					_errorconnect = false;
					_reconnectCount = 0;
					socket_.async_handshake(boost::asio::ssl::stream_base::client,
						[this](const boost::system::error_code& ec){
							if (!ec)
							{
								Connected.Invoke(this);
								DoWrite();
							}
							else
							{
								std::cout << ec.message() << std::endl;
								WaitReconnect();
							}
						}
					); // End async_handshake.
				}
				else
				{
					std::cout << ec.message() << std::endl;
					WaitReconnect();
				}
			}
		); // End async_connect.
#else
		boost::asio::async_connect(socket_, endpoint_iterator,
            [this](const boost::system::error_code& ec, const tcp::resolver::iterator& it) {
				if (!ec)
				{
					_connected = true;
					_reconnectCount = 0;
					Connected.Invoke(this);
					DoWrite();
				}
				else
				{
					std::cout << ec.message() << std::endl;
					WaitReconnect();
				}
			}
		);
#endif
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}

	return true;
}


/**
* Handles
*/
void TcpClient::DoRead()
{
	BeginTimeout();

	boost::asio::async_read(socket_,
		boost::asio::buffer(inbound_header_, header_length),
		[this](boost::system::error_code ec, std::size_t bytes_transferred){
			timerTimeout_.cancel();

			// TODO. Error. Block. This don't work.
			if (ec)
			{
				const std::string text = ec.message();
				std::cout << text << std::endl;
				WaitReconnect();
				return;
			}

			uint32 size = *reinterpret_cast<uint32*>(inbound_header_);
			inbound_data_.resize(size);

			if (size != 0)
			{
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					[this](const boost::system::error_code& ec, size_t bytes_transferred) {

						if (ec)
						{
							std::cout << ec.message() << std::endl;
							WaitReconnect();
							return;
						}

						uint8* data = reinterpret_cast<uint8*>(inbound_data_.data());
						ReadyRead.Invoke(data, bytes_transferred);

						DoWrite();
					}
				); // End body async_read.
			}
			else // TODO. Architecture bug.
			{
				DoWrite();
			}
		}
	); // End header async_read.
}

void TcpClient::DoWrite()
{
	// send commands and functions.
	std::lock_guard<std::mutex> lock(_mutex);

	if (_commands.empty())
	{
		waitTimer_.expires_from_now(boost::posix_time::milliseconds(200));
		waitTimer_.async_wait(std::bind(&TcpClient::DoWrite, this));
		return;
	}


	// Calculate all data size.
	uint32 allSize = 0;
	for (const TcpCommand& cmd : _commands)
	{
		allSize += cmd.Size;
	}

	outbound_data_.resize(sizeof(uint32) + allSize);
	uint8* data = &outbound_data_[0];

	MemoryBuffer buffer;
	buffer.Put(data, sizeof(uint32) + allSize);
	MemoryWriter out(buffer);
	out << allSize;

	for (const TcpCommand& cmd : _commands)
	{
		uint32 size = cmd.Size;
		out.ByteSerialize(cmd.Data, size);
	}

	_commands.clear();

	// Send data.
	boost::asio::async_write(socket_,
		boost::asio::buffer(outbound_data_),
		[this](boost::system::error_code ec, std::size_t) {

			if (ec)
			{
				std::cout << ec.message() << std::endl;
				WaitReconnect();
				return;
			}

			DoRead();
		}
	);

	//InfoStream::Write("Send: %f", _activity);
}

void TcpClient::BeginTimeout()
{
	timerTimeout_.cancel();
	timerTimeout_.expires_from_now(boost::posix_time::seconds(TIMEOUT_TIME));
	timerTimeout_.async_wait([this](const boost::system::error_code& ec){

		if (ec != boost::asio::error::operation_aborted)
		{
#ifdef WITH_SSL
			socket_.lowest_layer().close();
#else
			socket_.close();
#endif
			WaitReconnect();
		}
	});
}

} // End vnet.
