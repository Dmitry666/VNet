#include "VirtualNetworkPrivate.h"
#include "TcpClient.h"

#include <iostream>
#include <boost/exception/diagnostic_information.hpp>

namespace vnet {

//
void VirtualConnection::Connect(const std::string& address, const std::string& port)
{
	_address = address;
	_port = port;

	ServiceCommand command(RequestCommands::ClientConnect);
	command.Push("address", address);
	command.Push("port", port);
	command.Push("responsePort", _key);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << command;

	_vnp->SendCommand(buffer);
}

void VirtualConnection::Disconnect()
{
	ServiceCommand command(RequestCommands::ClientDisconnect);
	command.Push("address", _address);
	command.Push("port", _port);
	command.Push("responsePort", _key);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << command;

	_vnp->SendCommand(buffer);
}

void VirtualConnection::SendCommand(const uint8* bytes, uint32 nbBytes)
{
	ServiceCommand command(RequestCommands::SendCommand);
	command.Push("address", _address);
	command.Push("port", _port);
	command.Push("responsePort", _key);
	command.Push("data", Variant(bytes, nbBytes));

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << command;

	_vnp->SendCommand(buffer);
}

void VirtualConnection::SendCommand(const std::string& address, const std::string& port, const uint8* bytes, uint32 nbBytes)
{
	ServiceCommand command(RequestCommands::SendCommand);
	command.Push("address", address);
	command.Push("port", port);
	command.Push("responsePort", _key);
	command.Push("data", Variant(bytes, nbBytes));

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << command;

	_vnp->SendCommand(buffer);
}

//
VirtualNetworkPrivate::VirtualNetworkPrivate(const VirtualNetworkDesc& desc)
	: _address(desc.Address)
	, _port(desc.Port)
	, _secret(desc.Secret)
	, _virtualAddress(desc.VirtualAddress)

	, _tcpClient(nullptr)
	, _running(false)
{}

VirtualNetworkPrivate::~VirtualNetworkPrivate()
{
	_running = false;
	if (_thread.joinable())
		_thread.join();

	if (_tcpClient->IsConnected())
	{
		_tcpClient->Disconnect();
	}
	delete _tcpClient;

	//assert(_tcpClient != nullptr);
}

void VirtualNetworkPrivate::Initialize()
{
	try
	{
		_tcpClient = new TcpClient();
	}
	catch (std::exception e)
	{
		std::cout << "Unexpected exception caught in " <<
			BOOST_CURRENT_FUNCTION << std::endl <<
			boost::current_exception_diagnostic_information();

		delete _tcpClient;
		_tcpClient = nullptr;
		return;
	}
	

	_tcpClient->Connected = std::bind(&VirtualNetworkPrivate::OnConnected, this, std::placeholders::_1);
	_tcpClient->ConnectError = [this](TcpClient* client){
		this->ConnectError.Invoke(this);
	};
	_tcpClient->Disconnected = std::bind(&VirtualNetworkPrivate::OnDisconnected, this, std::placeholders::_1);
	_tcpClient->ReadyRead = std::bind(&VirtualNetworkPrivate::OnReadyRead, this, std::placeholders::_1, std::placeholders::_2);

	_tcpClient->Connect(_address, _port);
}

void VirtualNetworkPrivate::Release()
{
	if (_tcpClient != nullptr && _tcpClient->IsConnected())
	{
		ServiceCommand command(RequestCommands::Logout);
		MemoryBuffer buffer;
		MemoryWriter out(buffer);
		out << command;

		_tcpClient->Send(buffer.GetData(), buffer.GetSize());
	}
}

bool VirtualNetworkPrivate :: IsInitialized() const
{
	return _tcpClient != nullptr && _tcpClient->IsConnected();
}

void VirtualNetworkPrivate::Ping()
{
	ServiceCommand command(RequestCommands::Ping);
	//command.Push("data", Argument(bytes, nbBytes));

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << command;

	SendCommand(buffer);
}

VirtualConnection* VirtualNetworkPrivate::CreateConnection()
{
	std::string key = std::to_string(rand());

	VirtualConnection* connection = new VirtualConnection(key, this);
	_connections.insert(std::make_pair(key, connection));

	return connection;
}

VirtualConnection* VirtualNetworkPrivate::CreateConnection(const std::string& key)
{
	VirtualConnection* connection = new VirtualConnection(key, this);
	_connections.insert(std::make_pair(key, connection));

	return connection;
}

void VirtualNetworkPrivate::RemoveConnection(VirtualConnection* connection)
{
	auto it = std::find_if(_connections.begin(), _connections.end(), [connection](const std::pair<std::string, VirtualConnection*>& pair){
		return pair.second == connection;
	});

	if (it != _connections.end())
	{
		_connections.erase(it);
	}
}

void VirtualNetworkPrivate::SendCommand(const MemoryBuffer& buffer)
{
	_tcpClient->Send(buffer.GetData(), buffer.GetSize());
}

void VirtualNetworkPrivate::OnConnected(class TcpClient* client)
{
	ServiceCommand command(RequestCommands::Authorize);

	if (!_secret.empty())
		command.Push("secret", _secret);

	if (!_virtualAddress.empty())
		command.Push("virtualAddress", _virtualAddress);

	command.Push("localAddress", Variant("dmitry"));

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << command;

	client->Send(buffer.GetData(), buffer.GetSize());
}

void VirtualNetworkPrivate::OnDisconnected(class TcpClient* client)
{
	_running = false;
	if (_thread.joinable())
		_thread.join();
}

void VirtualNetworkPrivate::OnReadyRead(const uint8* bytes, uint32 nbBytes)
{
	MemoryReader in(MemoryBuffer(bytes, nbBytes));

	while (in.GetPosition() < in.GetSize())
	{
		ServiceCommand command;
		in >> command;

		ResponseCommands responseCommands = static_cast<ResponseCommands>(command.CommandKey);

		static std::map<ResponseCommands, std::function<void(VirtualNetworkPrivate&, const ServiceCommand&)>> commands = {
			std::make_pair(ResponseCommands::Authorizated, std::mem_fn(&VirtualNetworkPrivate::LoginedResponse)),
			std::make_pair(ResponseCommands::Logouted, std::mem_fn(&VirtualNetworkPrivate::LogoutedResponse)),
			std::make_pair(ResponseCommands::NewAddress, std::mem_fn(&VirtualNetworkPrivate::NewAddressResponse)),
			//std::make_pair(ResponseCommands::Status, std::mem_fn(&VirtualNetworkPrivate::LoginedResponse)),
			std::make_pair(ResponseCommands::Clients, std::mem_fn(&VirtualNetworkPrivate::ClientsResponse)),
			std::make_pair(ResponseCommands::Pong, std::mem_fn(&VirtualNetworkPrivate::PongResponse)),

			std::make_pair(ResponseCommands::ClientNotFound, std::mem_fn(&VirtualNetworkPrivate::ClientNotFoundResponse)),

			std::make_pair(ResponseCommands::ClientConnected, std::mem_fn(&VirtualNetworkPrivate::ConnectedResponse)),
			std::make_pair(ResponseCommands::ClientDisconnected, std::mem_fn(&VirtualNetworkPrivate::DisconnectedResponse)),
			std::make_pair(ResponseCommands::CommandSended, std::mem_fn(&VirtualNetworkPrivate::NothingResponse)),
			
			std::make_pair(ResponseCommands::ClientConnect, std::mem_fn(&VirtualNetworkPrivate::ConnectResponse)),
			std::make_pair(ResponseCommands::ClientDisconnect, std::mem_fn(&VirtualNetworkPrivate::DisconnectResponse)),

			std::make_pair(ResponseCommands::NoCommands, std::mem_fn(&VirtualNetworkPrivate::NothingResponse)),
			std::make_pair(ResponseCommands::Command, std::mem_fn(&VirtualNetworkPrivate::CommandResponse)),
			//std::make_pair(ResponseCommands::Commands, std::mem_fn(&VirtualNetworkPrivate::CommandsResponse)),

			std::make_pair(ResponseCommands::AccessDenied, std::mem_fn(&VirtualNetworkPrivate::NothingResponse))
			
		};
		
		auto it = commands.find(responseCommands);
		if (it != commands.end())
		{
			it->second(*this, command);
		}
		else
		{
			std::cout << "Command not found: " << command.CommandKey << std::endl;
		}
	}
}

void VirtualNetworkPrivate::RequestCommand()
{
	ServiceCommand command(RequestCommands::ReadCommands);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << command;

	SendCommand(buffer);
}

void VirtualNetworkPrivate::NothingResponse(const ServiceCommand& command)
{
	// Nothing.
}

void VirtualNetworkPrivate::LoginedResponse(const ServiceCommand& command)
{
	const PTree* keyArgument = command["key"];
	const PTree* vaArgument = command["address"];

	VirtualNetworkInfoPrivate vnip;
	vnip.Key = keyArgument != nullptr ? keyArgument->Data.AsString() : "";
	vnip.VirtualAddress = vaArgument != nullptr ? vaArgument->Data.AsString() : "";

	Authorizated.Invoke(this, vnip);

	// Start request command thread.
	_running = true;
	_thread = std::thread([this](){

		do
		{
			//Ping();
			RequestCommand();

			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		} while (_running);
	});
}

void VirtualNetworkPrivate::LogoutedResponse(const ServiceCommand& command)
{
	_running = false;
	if (_thread.joinable())
		_thread.join();

	Logouted.Invoke(this);
}

void VirtualNetworkPrivate::NewAddressResponse(const ServiceCommand& command)
{

}

void VirtualNetworkPrivate::ClientsResponse(const ServiceCommand& command)
{
}

void VirtualNetworkPrivate::PongResponse(const ServiceCommand& command)
{
	// ?
}

void VirtualNetworkPrivate::ClientNotFoundResponse(const ServiceCommand& command)
{
	VirtualClientInfo info;
	const PTree* vpArgument = command["responsePort"];
	if (vpArgument != nullptr)
	{
		const std::string& port = vpArgument->Data.AsString();
		auto it = _connections.find(port);
		if (it != _connections.end())
		{
			VirtualConnection* connection = it->second;
			connection->ConnectError.Invoke(info);
		}
		else
		{
			std::cout << "ClientNotFoundResponse: Connection not found for '" << port << "'." << std::endl;
		}
	}
	else
	{
		std::cout << "ClientNotFoundResponse: Not found 'port' argument." << std::endl;
	}

	// std::cout << "ClientNotFoundResponse" << std::endl;
}

void VirtualNetworkPrivate::ConnectedResponse(const ServiceCommand& command)
{
	// TODO. Work brain.
	VirtualClientInfo info;
	const PTree* vpArgument = command["port"];
	if (vpArgument != nullptr)
	{
		const std::string& port = vpArgument->Data.AsString();
		auto it = _connections.find(port);
		if (it != _connections.end())
		{
			VirtualConnection* connection = it->second;
			connection->Connected.Invoke(info);
			connection->SetConnected(true);
		}
		else
		{
			std::cout << "ConnectedResponse: Connection not found for '" << port << "'." << std::endl;
		}
	}
	else
	{
		std::cout << "ConnectedResponse: Not found 'port' argument." << std::endl;
	}


	// !
	// ConnectedToClient.Invoke(info);
}

void VirtualNetworkPrivate::DisconnectedResponse(const ServiceCommand& command)
{
	// TODO. Work brain.
	VirtualClientInfo info;
	const PTree* vpArgument = command["port"];
	if (vpArgument != nullptr)
	{
		const std::string& port = vpArgument->Data.AsString();
		auto it = _connections.find(port);
		if (it != _connections.end())
		{
			VirtualConnection* connection = it->second;
			connection->Disconnected.Invoke(info);
			connection->SetConnected(false);
		}
		else
		{
			std::cout << "DisconnectedResponse: Connection not found for '" << port << "'." << std::endl;
		}
	}
	else
	{
		std::cout << "DisconnectedResponse: Not found 'port' argument." << std::endl;
	}

	// !
	//DisconnectedClient.Invoke(info);
}

void VirtualNetworkPrivate::ConnectResponse(const ServiceCommand& command)
{
	//const PTree* laArgument = command["localAddress"];
	const PTree* vaArgument = command["address"];
	const PTree* vpArgument = command["port"];
	const PTree* rvpArgument = command["responsePort"];

	VirtualClientInfo vci;
	//vci.LocalAddress = laArgument != nullptr ? laArgument->Data.AsString() : "";
	vci.VirtualAddress = vaArgument != nullptr ? vaArgument->Data.AsString() : "";
	vci.VirtualPort = rvpArgument != nullptr ? rvpArgument->Data.AsString() : "";

	if (vpArgument != nullptr)
	{
		const std::string& port = vpArgument->Data.AsString();
		auto it = _connections.find(port);
		if (it != _connections.end())
		{
			VirtualConnection* connection = it->second;
			connection->NewClient.Invoke(vci);
		}
		else
		{
			std::cout << "ConnectResponse: Connection not found for '" << port << "'." << std::endl;
		}
	}
	else
	{
		std::cout << "ConnectResponse: Not found 'port' argument." << std::endl;
	}
	
	// TODO. Remove
	//VirtualClientConnect.Invoke(vci);
}

void VirtualNetworkPrivate::DisconnectResponse(const ServiceCommand& command)
{
	//const PTree* laArgument = command["localAddress"];
	const PTree* vaArgument = command["address"];
	const PTree* vpArgument = command["port"];
	const PTree* rvpArgument = command["responsePort"];

	VirtualClientInfo vci;
	//vci.LocalAddress = laArgument != nullptr ? laArgument->Data.AsString() : "";
	vci.VirtualAddress = vaArgument != nullptr ? vaArgument->Data.AsString() : "";
	vci.VirtualPort = rvpArgument != nullptr ? rvpArgument->Data.AsString() : "";

	if (vpArgument != nullptr)
	{
		const std::string& port = vpArgument->Data.AsString();
		auto it = _connections.find(port);
		if (it != _connections.end())
		{
			VirtualConnection* connection = it->second;
			connection->RemoveClient.Invoke(vci);
		}
		else
		{
			std::cout << "DisconnectResponse: Connection not found for '" << port << "'." << std::endl;
		}
	}
	else
	{
		std::cout << "DisconnectResponse: Not found 'port' argument." << std::endl;
	}

	// TODO. Remove
	//VirtualClientDisconnect.Invoke(vci);
}

void VirtualNetworkPrivate::NoCommandsResponse(const ServiceCommand& command)
{

}

void VirtualNetworkPrivate::CommandResponse(const ServiceCommand& command)
{
	const PTree* vaArgument = command["address"];
	const PTree* vpArgument = command["port"];
	const PTree* rvpArgument = command["responsePort"];
	const PTree* dataArgument = command["data"];

	VirtualClientInfo vci;
	//vci.LocalAddress = laArgument != nullptr ? laArgument->Data.AsString() : "";
	vci.VirtualAddress = vaArgument != nullptr ? vaArgument->Data.AsString() : "";
	vci.VirtualPort = rvpArgument != nullptr ? rvpArgument->Data.AsString() : "";

	if (dataArgument != nullptr && vpArgument != nullptr)
	{
		const std::string& port = vpArgument->Data.AsString();
		auto it = _connections.find(port);
		if (it != _connections.end())
		{
			VirtualConnection* connection = it->second;
			const std::vector<uint8> &data = dataArgument->Data.AsBytes();
			connection->NewCommand.Invoke(vci, data.data(), data.size());
		}
		else
		{
			std::cout << "CommandResponse: Connection not found for '" << port << "'." << std::endl;
		}
	}
	else
	{
		std::cout << "CommandResponse: Not found 'port' argument." << std::endl;
	}

	// TODO. Remove.
	if (dataArgument != nullptr)
	{
		//const std::vector<uint8> &data = dataArgument->Data.AsBytes();
		//VirtualClientCommand.Invoke(vci, data.data(), data.size());
	}
}

void VirtualNetworkPrivate::CommandsResponse(const ServiceCommand& command)
{

}

} // End vnet.