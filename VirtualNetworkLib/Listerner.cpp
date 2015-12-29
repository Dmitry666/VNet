#include "Listerner.h"

#include "TcpServer.h"

#include "VirtualNetwork.h"
#include "VirtualNetworkPrivate.h"

#include "VirtualSocket.h"
#include "DirectSocket.h"

#include <functional>

namespace vnet {

Listerner::Listerner(int32 port, VirtualNetwork* virtualNetwork)
	: _address("0.0.0.0")
	, _port(port)

	, _virtualNetworkPrivate(virtualNetwork->_virtualNetworkPrivate->shared_from_this())
	, _tcpServer(nullptr)
{
	_tcpServer = new TcpServer(_address, std::to_string(port));
	_virtualConnection = _virtualNetworkPrivate->CreateConnection(std::to_string(port));
}

Listerner::Listerner(const std::string& address, int32 port, VirtualNetwork* virtualNetwork)
	: _address(address)
	, _port(port)

	, _virtualNetworkPrivate(virtualNetwork->_virtualNetworkPrivate->shared_from_this())
	, _tcpServer(nullptr)
{
	_tcpServer = new TcpServer(address, std::to_string(port));
	_virtualConnection = _virtualNetworkPrivate->CreateConnection(std::to_string(port));
}

Listerner::~Listerner()
{
	_virtualNetworkPrivate->RemoveConnection(_virtualConnection);

	for (Socket* socket : _sockets)
		delete socket;
	_sockets.clear();

	delete _tcpServer;
}

void Listerner::Start()
{
	if (_virtualNetworkPrivate != nullptr)
	{
		_virtualConnection->NewClient = std::bind(&Listerner::OnVirtualClientConnected, this, std::placeholders::_1);
		_virtualConnection->RemoveClient = std::bind(&Listerner::OnVirtualClientDisconnected, this, std::placeholders::_1);
		_virtualConnection->NewCommand = std::bind(&Listerner::OnVirtualClientCommand, this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3);

		//_virtualNetworkPrivate->VirtualClientConnect = std::bind(&Listerner::OnVirtualClientConnected, this, std::placeholders::_1);
		//_virtualNetworkPrivate->VirtualClientDisconnect = std::bind(&Listerner::OnVirtualClientDisconnected, this, std::placeholders::_1);
		//_virtualNetworkPrivate->VirtualClientCommand = std::bind(&Listerner::OnVirtualClientCommand, this,
		//	std::placeholders::_1,
		//	std::placeholders::_2,
		//	std::placeholders::_3);
	}

	_tcpServer->NewConnection = std::bind(&Listerner::OnDirectClientConnected, this, std::placeholders::_1);
	_tcpServer->RemoveConnection = std::bind(&Listerner::OnDirectClientDisconnected, this, std::placeholders::_1);
	_tcpServer->Start();
}

void Listerner::Stop()
{
	_tcpServer->Stop();
}

/*
Socket* Listerner::AcceptSocket()
{
	//_tcpServer->Accept();
	//Socket
}

void Listerner::AcceptSocketAsync(Delegate<Socket*> delegate)
{

}

bool Listerner::IsActive()
{}
*/

void Listerner::OnDirectClientConnected(TcpConnection* connection)
{
	DirectSocket* socket = new DirectSocket(connection);
	_sockets.push_back(socket);

	Connected.Invoke(this, socket);
}

void Listerner::OnDirectClientDisconnected(TcpConnection* connection)
{
	auto it = std::find_if(_sockets.begin(), _sockets.end(), [connection](Socket* socket){
		DirectSocket* directSocket = dynamic_cast<DirectSocket*>(socket);
		return directSocket != nullptr ? (directSocket->GetConnection() == connection) : false;
	});

	if (it == _sockets.end())
	{
		//std::cout
		return;
	}

	// assert(it != _sockets.end());

	Socket* socket = *it;
	Disconnected.Invoke(this, socket);
}

void Listerner::OnVirtualClientConnected(const VirtualClientInfo& vci)
{
	VirtualSocket* socket = new VirtualSocket(_virtualConnection, vci);
	_sockets.push_back(socket);

	Connected.Invoke(this, socket);
}

void Listerner::OnVirtualClientDisconnected(const VirtualClientInfo& vci)
{
	auto it = std::find_if(_sockets.begin(), _sockets.end(), [vci](Socket* socket){
		VirtualSocket* virtualSocket = dynamic_cast<VirtualSocket*>(socket);
		return virtualSocket != nullptr ? (virtualSocket->GetAddress() == vci.VirtualAddress) : false;
	});

	if (it == _sockets.end())
	{
		//std::cout
		return;
	}

	//assert(it != _sockets.end());

	Socket* socket = *it;

	Disconnected.Invoke(this, socket);
}

void Listerner::OnVirtualClientCommand(const VirtualClientInfo& vci, const uint8* bytes, uint32 nbBytes)
{
	auto it = std::find_if(_sockets.begin(), _sockets.end(), [vci](Socket* socket){
		VirtualSocket* virtualSocket = dynamic_cast<VirtualSocket*>(socket);
		return virtualSocket != nullptr ? (virtualSocket->GetAddress() == vci.VirtualAddress) : false;
	});

	if (it == _sockets.end())
	{
		//std::cout
		return;
	}
	//assert(it != _sockets.end());

	Socket* socket = *it;

	VirtualSocket* virtualSocket = reinterpret_cast<VirtualSocket*>(socket);
	virtualSocket->OnNewRequest(vci, bytes, nbBytes);
}

} // End vnet.