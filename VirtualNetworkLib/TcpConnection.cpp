#include "TcpConnection.h"
#include "TcpConnectionManager.h"

#include <iostream>

namespace vnet {

TcpConnection::TcpConnection(
#ifdef WITH_SSL
	boost::asio::io_service& io_service_,
	boost::asio::ssl::context& context_,
#else
	connection_socket socket,
#endif
	TcpConnectionManager& manager)
#ifdef WITH_SSL
	: socket_(io_service_, context_)
#else
	: socket_(std::move(socket))
#endif
    , connection_manager_(manager)
{}

void TcpConnection::Start()
{
    //boost::asio::ip::tcp::no_delay option(true);
	auto self(shared_from_this());

#ifdef WITH_SSL
	//socket_.lowest_layer().set_option(option);
	socket_.async_handshake(boost::asio::ssl::stream_base::server,
		[this, self](const boost::system::error_code& ec)
		{		
			if (!ec)
			{
				DoRead();
			}
			else
			{
				const std::string text = ec.message();
				std::cout << text << std::endl;
				connection_manager_.Stop(self);
			}
		});
#else
	//socket_.set_option(option);  
	DoRead();
#endif
}

void TcpConnection::Stop()
{}

void TcpConnection::Send(const uint8* bytes, uint32 nbBytes)
{
	// TODO. ????? Empty?
	std::lock_guard<std::mutex> _lock(_mutex);
	_commands.push_back(TcpCommand(bytes, nbBytes));
}

void TcpConnection::DoRead()
{
    auto self(shared_from_this());
	socket_.async_read_some(boost::asio::buffer(inbound_header_, header_length),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
				uint32 size = *reinterpret_cast<uint32*>(inbound_header_);
	
				inbound_data_.resize(size);
				boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
					[this, self](boost::system::error_code ec, std::size_t bytes_transferred) {

						//
						if (!ec)
						{
							const uint8* data = inbound_data_.data();
							NewRequest.Invoke(data, bytes_transferred);

							DoWrite();
						}
						else //if (ec != boost::asio::error::operation_aborted)
						{
							std::cout << ec.message() << std::endl;
							connection_manager_.Stop(self);
						}
					}
				); // End async_read body.
            }
            else //if (ec != boost::asio::error::operation_aborted)
            {
				std::cout << ec.message() << std::endl;
				connection_manager_.Stop(self);
            }
        }
	); // End async_read_some header.
}

void TcpConnection::DoWrite()
{
	auto self(shared_from_this());

#if 0  
    boost::asio::async_write(socket_, reply_.to_buffers(),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                // Initiate graceful connection closure.
                boost::system::error_code ignored_ec;
                socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
            }

            if (ec != boost::asio::error::operation_aborted)
            {
                connection_manager_.stop(shared_from_this());
            }
        });
#endif

	//
	// send commands and functions.
	std::lock_guard<std::mutex> lock(_mutex);

	//if (_commands.empty())
	//{
	//	waitTimer_.expires_from_now(boost::posix_time::milliseconds(200));
	//	waitTimer_.async_wait(std::bind(&TcpClient::DoWrite, this));
	//	return;
	//}


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
				//WaitReconnect();
				return;
			}

			DoRead();
		}
	);
}

} // namespace vnet.

