#include "VirtualSocket.h"
#include "VirtualNetworkPrivate.h"

#include <iostream>

namespace vnet {

VirtualSocket::VirtualSocket(VirtualConnection* conenction)
	: Socket()
	//, _vnp(vnp)
	, _virtualConnection(conenction)
{}

VirtualSocket::VirtualSocket(VirtualConnection* conenction, const VirtualClientInfo& vci)
	: Socket()
	//, _vnp(vnp)
	, _virtualConnection(conenction)
{
	_address = vci.VirtualAddress;
	_port = vci.VirtualPort;
}

VirtualSocket::~VirtualSocket()
{}

void VirtualSocket::Connect(const std::string& address, int32 port)
{
	if (_virtualConnection == nullptr)
	{
		std::cout << "Socket not valid." << std::endl;
		return;
	}
	_address = address;

	_virtualConnection->Connected = [this](const VirtualClientInfo& info){
		Connected.Invoke(this);
		_virtualConnection->NewCommand += std::bind(&VirtualSocket::OnNewRequest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	};
	_virtualConnection->ConnectError = [this](const VirtualClientInfo& info){
		ConnectError.Invoke(this);
	};

	_virtualConnection->Connect(address, std::to_string(port));
}

void VirtualSocket::Disconnect()
{
	if (_address.empty())
	{
		std::cout << "Socket not valid." << std::endl;
		return;
	}

	_virtualConnection->Disconnected = [this](const VirtualClientInfo& info){
		Disconnected.Invoke(this);
		_address.clear();
	};

	_virtualConnection->Disconnect();
}

void VirtualSocket::Send(const uint8* bytes, int32 nbBytes)
{
	if (_address.empty())
	{
		std::cout << "Socket not valid." << std::endl;
		return;
	}

	if (!_address.empty() && !_port.empty())
	{
		_virtualConnection->SendCommand(_address, _port, bytes, nbBytes);
	}
	else
	{
		_virtualConnection->SendCommand(bytes, nbBytes);
	}
}

void VirtualSocket::Read(uint8* bytes, int32 nbBytes)
{
	//!
}

bool VirtualSocket::IsConnected() const
{
	return _virtualConnection != nullptr && _virtualConnection->IsConnected();
}

void VirtualSocket::OnNewRequest(const VirtualClientInfo&, const uint8* bytes, int32 nbBytes)
{
	NewRequest.Invoke(bytes, nbBytes);
}

} // End vnet.