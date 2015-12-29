#include "tcp_server.hpp"
#include <iostream>

namespace vnet {

tcp_server::tcp_server(const std::string& address, const std::string& port, request_handler& request_handler)
    : io_service_()
	, context_(boost::asio::ssl::context::sslv23)
    , signals_(io_service_)
    , acceptor_(io_service_)
    , connection_manager_()
#ifdef WITH_SSL
	//, socket_(io_service_, context_)
#else
    , socket_(io_service_)
#endif
	, request_handler_(request_handler)
{
#ifdef WITH_SSL
	context_.set_options(
		boost::asio::ssl::context::default_workarounds
		| boost::asio::ssl::context::no_sslv2
		| boost::asio::ssl::context::single_dh_use);
	//context_.set_password_callback(boost::bind(&TcpServer::get_password, this));
	context_.use_certificate_chain_file("server.crt");
	context_.use_private_key_file("server.key", boost::asio::ssl::context::pem);
	context_.use_tmp_dh_file("dh512.pem");
#endif


    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

    do_await_stop();

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    do_accept();
}

void tcp_server::start()
{
	thread_ = std::thread([this](){

		// The io_service::run() call will block until all asynchronous operations
		// have finished. While the server is running, there is always at least one
		// asynchronous operation outstanding: the asynchronous accept call waiting
		// for new incoming connections.
		io_service_.run();
	});
}

void tcp_server::stop()
{
	io_service_.stop();
	thread_.join();
}

void tcp_server::do_accept()
{
#ifdef WITH_SSL
	connection_ptr conn = std::make_shared<connection>(io_service_, context_, connection_manager_, request_handler_);
#endif

    acceptor_.async_accept(
#ifdef WITH_SSL
		conn->socket().lowest_layer(),// socket_.lowest_layer(),
#else
		socket_,
#endif
		[this, conn](boost::system::error_code ec)
        {
            // Check whether the server was stopped by a signal before this
            // completion handler had a chance to run.
            if (!acceptor_.is_open())
            {
                return;
            }

            if (!ec)
            {
				std::cout << "new accept..." << std::endl;
#ifdef WITH_SSL
				connection_manager_.start(conn);
#else
                connection_manager_.start(std::make_shared<connection>(
					std::move(socket_), connection_manager_, request_handler_));
#endif
            }

            do_accept();
        }
	); // End async_accept.
}

void tcp_server::do_await_stop()
{
    signals_.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/)
        {
            // The server is stopped by cancelling all outstanding asynchronous
            // operations. Once all operations have finished the io_service::run()
            // call will exit.
            acceptor_.close();
            connection_manager_.stop_all();
        }
	);
}

} // namespace vnet.