#include "Client.h"
#include "VirtualNetwork.h"
#include "VirtualNetworkPrivate.h"

#include "DirectSocket.h"
#include "VirtualSocket.h"

namespace vnet {


Client::Client(VirtualNetwork* virtualNetwork)
	: _socket(nullptr)
	//, _virtualNetworkPrivate(virtualNetwork->_virtualNetworkPrivate->shared_from_this())
	, _virtualConnection(nullptr)
{

	if (virtualNetwork != nullptr && virtualNetwork->_virtualNetworkPrivate != nullptr)
	{
		_virtualNetworkPrivate = virtualNetwork->_virtualNetworkPrivate->shared_from_this();
	}

	if (_virtualNetworkPrivate != nullptr && _virtualNetworkPrivate->IsInitialized())
	{
		_virtualConnection = _virtualNetworkPrivate->CreateConnection();
	}
}

Client::~Client()
{
	if (_virtualConnection != nullptr)
	{
		_virtualNetworkPrivate->RemoveConnection(_virtualConnection);
	}
	
	delete _socket;
}

void Client::Connect(const std::string& address, uint32 port)
{
	delete _socket;
	_socket = nullptr;

	auto virtualConnect = [this](const std::string& address, uint32 port){
		VirtualSocket* virtualSocket = new VirtualSocket(_virtualConnection);
		virtualSocket->Connected += [this](VirtualSocket* socket){
			_socket = socket;

			socket->Disconnected += [this](VirtualSocket* socket){
				Disconnected.Invoke(this);
			};

			socket->NewRequest += [this](const uint8* bytes, uint32 nbBytes){
				NewRequest.Invoke(bytes, nbBytes);
			};

			Connected.Invoke(this);
		};

		virtualSocket->ConnectError += [this](VirtualSocket* socket){
			ConnectError.Invoke(this);
		};

		virtualSocket->Connect(address, port);
	};

	auto directConnect = [this, virtualConnect](const std::string& address, uint32 port){

		DirectSocket* socket = new DirectSocket();
		socket->Connected += [this](DirectSocket* socket){
			_socket = socket;

			socket->Disconnected += [this](DirectSocket* socket){
				Disconnected.Invoke(this);
			};

			_socket->NewRequest += [this](const uint8* bytes, uint32 nbBytes){
				NewRequest.Invoke(bytes, nbBytes);
			};

			Connected.Invoke(this);
		};

		socket->ConnectError += [this, virtualConnect, address, port](DirectSocket* socket){
			if (_virtualConnection != nullptr)
			{
				virtualConnect(address, port);
			}		
			else
			{
				// No virtual connection.
				ConnectError.Invoke(this);
			}
		};

		socket->Connect(address, port);
	};

	if (address.size() > strlen("vbtp://"))
	{ 
		size_t found = address.find_first_of("vbtp://");
		if (found == 0) //found != std::string::npos)
		{
			std::string virtualAddress = std::string(address.begin() + strlen("vbtp://"), address.end());
			if (_virtualConnection != nullptr)
			{
				virtualConnect(virtualAddress, port);
			}
			else
			{
				// No virtual connection.
				ConnectError.Invoke(this);
			}

			return;
		}
	}

	directConnect(address, port);
}

void Client::Disconnect()
{
	if (_socket != nullptr)
	{
		_socket->Disconnect();
	}
}

void Client::Send(uint8* bytes, uint32 nbBytes)
{
	if (_socket != nullptr)
	{
		_socket->Send(bytes, nbBytes);
	}
}

bool Client::IsConnected() const
{
	return _socket != nullptr && _socket->IsConnected();
}

} // End vnet.