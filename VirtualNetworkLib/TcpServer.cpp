#include "TcpServer.h"

namespace vnet {

TcpServer::TcpServer(const std::string& address, const std::string& port)
    : io_service_()
#ifdef WITH_SSL
	, context_(boost::asio::ssl::context::sslv23)
#endif
    , signals_(io_service_)
    , acceptor_(io_service_)
    , connection_manager_()

#ifdef WITH_SSL
	// , socket_(io_service_, context_)
#else
    , socket_(io_service_)
#endif
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

    DoAwaitStop();

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    DoAccept();
}

TcpServer::~TcpServer()
{
	io_service_.stop();
	if (thread_.joinable())
		thread_.join();
}

void TcpServer::Start()
{
	thread_ = std::thread([this](){

		// The io_service::run() call will block until all asynchronous operations
		// have finished. While the server is running, there is always at least one
		// asynchronous operation outstanding: the asynchronous accept call waiting
		// for new incoming connections.
		io_service_.run();
	});
}

void TcpServer::Stop()
{
	io_service_.stop();
	if (thread_.joinable())
		thread_.join();
}

void TcpServer::DoAccept()
{
#ifdef WITH_SSL
	TcpConnectionPtr connection = std::make_shared<TcpConnection>(io_service_, context_, connection_manager_);
#endif

    acceptor_.async_accept(
#ifdef WITH_SSL
		connection->socket().lowest_layer(),// socket_.lowest_layer(),
#else
		socket_,
#endif
		[this, connection](boost::system::error_code ec)
        {
            // Check whether the server was stopped by a signal before this
            // completion handler had a chance to run.
            if (!acceptor_.is_open())
            {
                return;
            }

            if (!ec)
            {
#ifndef WITH_SSL
				TcpConnectionPtr connection = std::make_shared<TcpConnection>(std::move(socket_), connection_manager_);		
#endif
				connection_manager_.Start(connection);
				NewConnection.Invoke(connection.get());
            }

            DoAccept();
        }
	);
}

void TcpServer::DoAwaitStop()
{
    signals_.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/)
        {
            // The server is stopped by cancelling all outstanding asynchronous
            // operations. Once all operations have finished the io_service::run()
            // call will exit.
            acceptor_.close();
            connection_manager_.StopAll();
        }
	);
}

} // namespace vnet.