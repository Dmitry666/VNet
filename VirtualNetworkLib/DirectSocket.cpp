#include "DirectSocket.h"

#include "TcpClient.h"
#include "TcpConnection.h"

#include <functional>

namespace vnet {


DirectSocket::DirectSocket()
	: Socket()
	, _tcpClient(nullptr)
	, _tcpConnection(nullptr)
	, _running(false)
{}

DirectSocket::DirectSocket(TcpConnection* tcpConnection)
	: Socket()
	, _tcpClient(nullptr)
	, _tcpConnection(tcpConnection)
	, _running(false)
{
	_tcpConnection->NewRequest += std::bind(&DirectSocket::OnNewRequest, this, std::placeholders::_1, std::placeholders::_2);
}

DirectSocket::~DirectSocket()
{
	delete _tcpClient;
}

void DirectSocket::Connect(const std::string& address, int32 port)
{
	delete _tcpClient;
	_tcpClient = nullptr;

	_tcpClient = new TcpClient(false);
	_tcpClient->Connected = [this](TcpClient* client){
		Connected.Invoke(this);

		_tcpClient->Disconnected = [this](TcpClient* client){
			Disconnected.Invoke(this);
		};

		_tcpClient->ReadyRead += std::bind(&DirectSocket::OnNewRequest, this, std::placeholders::_1, std::placeholders::_2);

		_running = true;
		_thread = std::thread([this](){

			do
			{
				_tcpClient->Ping();

				std::this_thread::sleep_for(std::chrono::milliseconds(300));
			} while (_running);
		});
	};

	_tcpClient->ConnectError = [this](TcpClient* client){
		ConnectError.Invoke(this);
	};

	_tcpClient->Connect(address, port);
}

void DirectSocket::Disconnect()
{
	_running = false;
	if (_thread.joinable())
		_thread.join();

	if (_tcpClient != nullptr)
	{ 
		_tcpClient->Disconnect();
	}
}

void DirectSocket::Send(const uint8* bytes, int32 nbBytes)
{
	if (_tcpClient != nullptr)
	{
		_tcpClient->Send(bytes, nbBytes);
		return;
	}

	if (_tcpConnection != nullptr)
	{
		_tcpConnection->Send(bytes, nbBytes);
	}
}

void DirectSocket::Read(uint8* bytes, int32 nbBytes)
{
}

bool DirectSocket::IsConnected() const
{
	return (_tcpClient != nullptr && _tcpClient->IsConnected()) || (_tcpConnection != nullptr);
}

void DirectSocket::OnNewRequest(const uint8* bytes, int32 nbBytes)
{
	NewRequest.Invoke(bytes, nbBytes);
}

} // End vnet.