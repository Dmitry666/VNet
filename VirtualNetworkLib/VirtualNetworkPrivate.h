#pragma once

#include "Common.h"
#include "CommonPrivate.h"
#include "VirtualNetwork.h" 

#include <thread>

namespace vnet {

/**
 * @brief Virtual client info struct.
 */


/**
 * @brief Virtual network info struct.
 */
struct VirtualNetworkInfoPrivate
{
	std::string Key;
	std::string VirtualAddress;
};

/**
 * @brief Virtual Connection
 */
class VirtualConnection
{
public:
	VirtualConnection(const std::string& key, class VirtualNetworkPrivate* vnp)
		: _key(key)
		, _connected(false)
		, _vnp(vnp)
	{}

	VirtualConnection(const VirtualConnection&) = delete;
	VirtualConnection& operator = (const VirtualConnection&) = delete;

	void Connect(const std::string& address, const std::string& port);
	void Disconnect();

	void SendCommand(const uint8* bytes, uint32 nbBytes);
	void SendCommand(const std::string& address, const std::string& port, const uint8* bytes, uint32 nbBytes);

	bool IsConnected() const
	{
		return _connected;
	}

	void SetConnected(bool c)
	{
		_connected = c;
	}

public:
	Delegate<const VirtualClientInfo&> Connected;
	Delegate<const VirtualClientInfo&> ConnectError;
	Delegate<const VirtualClientInfo&> Disconnected;

	Delegate<const VirtualClientInfo&> NewClient;
	Delegate<const VirtualClientInfo&> RemoveClient;
	Delegate<const VirtualClientInfo&, const uint8*, uint32> NewCommand;

private:
	std::string _key;
	std::string _address;
	std::string _port;

	bool _connected;

	class VirtualNetworkPrivate* _vnp;
};

/**
 * @brief Private class for virtual network.
 */
class VirtualNetworkPrivate : public std::enable_shared_from_this<VirtualNetworkPrivate>
{
public:
	VirtualNetworkPrivate(const VirtualNetworkDesc& desc);
	virtual ~VirtualNetworkPrivate();
	
	void Initialize();
	void Release();
	bool IsInitialized() const;

	void Ping();

	VirtualConnection* CreateConnection();
	VirtualConnection* CreateConnection(const std::string& key);
	void RemoveConnection(VirtualConnection* connection);

	void SendCommand(const MemoryBuffer& buffer);

public:
	Delegate<VirtualNetworkPrivate*, const VirtualNetworkInfoPrivate&> Authorizated;
	Delegate<VirtualNetworkPrivate*> ConnectError;	
	Delegate<VirtualNetworkPrivate*> Logouted;

private:
	void OnConnected(class TcpClient* client);
	void OnDisconnected(class TcpClient* client);
	void OnReadyRead(const uint8* bytes, uint32 nbBytes);

private:
	void RequestCommand();

	void NothingResponse(const ServiceCommand& command);

	void LoginedResponse(const ServiceCommand& command);
	void LogoutedResponse(const ServiceCommand& command);
	void NewAddressResponse(const ServiceCommand& command);
	void ClientsResponse(const ServiceCommand& command);
	void PongResponse(const ServiceCommand& command);

	void ClientNotFoundResponse(const ServiceCommand& command);	

	void ConnectedResponse(const ServiceCommand& command);
	void DisconnectedResponse(const ServiceCommand& command);

	void ConnectResponse(const ServiceCommand& command);
	void DisconnectResponse(const ServiceCommand& command);

	void NoCommandsResponse(const ServiceCommand& command);
	void CommandResponse(const ServiceCommand& command);
	void CommandsResponse(const ServiceCommand& command);

private:
	std::string _address;
	uint32 _port;
	std::string _secret;
	std::string _virtualAddress;

	class TcpClient* _tcpClient;

	bool _running;
	std::thread _thread;

	std::map<std::string, VirtualConnection*> _connections;
};

} // End vnet.