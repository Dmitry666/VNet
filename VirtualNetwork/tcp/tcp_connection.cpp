#include "tcp_connection.hpp"
#include "tcp_connection_manager.hpp"

#include <iostream>

namespace vnet {

struct membuf : std::streambuf
{
	membuf(char* begin, char* end) {
		this->setg(begin, begin, end);
	}
};

connection::connection(
#ifdef WITH_SSL
	boost::asio::io_service& io_service_,
	boost::asio::ssl::context& context_,
#else
	connection_socket socket,
#endif
	connection_manager& manager, request_handler& handler)
#ifdef WITH_SSL
	: socket_(io_service_, context_)
#else
	: socket_(std::move(socket))
#endif
	, connection_manager_(manager)
    , request_handler_(handler)
{}

void connection :: start()
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
				do_read();
			}
			else
			{
				std::cout << "async_handshake: " << ec.message() << std::endl;

				request_handler_.handle_remove_connection(request_.key);
				connection_manager_.stop(self);
			}
		});
#else
	//socket_.set_option(option);   
	do_read();

	/*
    socket_.async_read_some(boost::asio::buffer(buffer_, 4),
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
        {
            if (!ec)
            {
				// Read token, Network address.
				// Send address.
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
				connection_manager_.stop(self);
            }
        });
	*/
#endif
}

void connection::do_read()
{
    auto self(shared_from_this());

	buffer_.resize(4);
    socket_.async_read_some(boost::asio::buffer(buffer_, 4),
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
        {
            if (!ec)
            {
				uint32_t size;
				memcpy(&size, buffer_.data(), sizeof(uint32_t));
	
				buffer_.resize(size);
				boost::asio::async_read(socket_,
					boost::asio::buffer(buffer_, size),
					[this, self](boost::system::error_code error, std::size_t bytes_transferred)
					{
						const uint8* data = reinterpret_cast<uint8*>(buffer_.data());

						MemoryBuffer buffer(data, bytes_transferred);;
						MemoryReader in(buffer);

						reply_.reset();

						while (in.GetPosition() < in.GetSize())
						{ 
							in >> request_;
							request_handler_.handle_request(request_, reply_);
							request_.key = reply_.key;
						}

						do_write();
					});
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
				std::cout << "do_read: " << ec.message() << std::endl;

				request_handler_.handle_remove_connection(request_.key);
				connection_manager_.stop(self);
            }
        }
	);
}

void connection::stop()
{}

void connection::do_write()
{
	auto self(shared_from_this());

    boost::asio::async_write(socket_, reply_.to_buffers(),
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
				do_read();
				return;
            }

            //if (ec != boost::asio::error::operation_aborted)
            //{
				std::cout << "do_write: " << ec.message() << std::endl;

				request_handler_.handle_remove_connection(request_.key);
				connection_manager_.stop(self);
            //}
        }
	);
}

} // namespace vnet.

