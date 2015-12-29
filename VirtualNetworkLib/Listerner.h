#pragma once

#include "Common.h"
#include "VirtualNetwork.h"

#include <vector>
#include "Delegate.h"


namespace vnet {

class Socket;
class Client;

/**
 * @brief Network listerner.
 */
class Listerner
{
public:
	/**
	 * @brief Constructor with port, default address and central virtual network.
	 */
	VNET_API Listerner(int32 port, class VirtualNetwork* virtualNetwork = nullptr);

	/**
	 * @brief Contructor with address, port and central virtual network.
	 */
	VNET_API Listerner(const std::string& address, int32 port, class VirtualNetwork* virtualNetwork = nullptr);

	/**
	 * @brief Destructor.
	 */
	VNET_API virtual ~Listerner();

	/**
	 * @brief Start accept connections in thread.
	 */
	VNET_API void Start();

	/**
	 * @brief Stop listerner thread.
	 */
	VNET_API void Stop();

	// Inlines.
	_inline const std::string& GetAddress() const { return _address; }
	_inline int32 GetPort() const { return _port; }

public:
	Delegate<Listerner*, Socket*> Connected;	//! New connection signal.
	Delegate<Listerner*, Socket*> Disconnected;	//! Connection removed signal.

	// private.
private:
	void OnDirectClientConnected(class TcpConnection* connection);
	void OnDirectClientDisconnected(class TcpConnection* connection);

	void OnVirtualClientConnected(const VirtualClientInfo& vci);
	void OnVirtualClientDisconnected(const VirtualClientInfo& vci);
	void OnVirtualClientCommand(const VirtualClientInfo& vci, const uint8* bytes, uint32 nbBytes);

private:
	std::string _address;
	int32 _port;

	std::vector<Socket*> _sockets;

private:
	std::shared_ptr<class VirtualNetworkPrivate> _virtualNetworkPrivate;
	class VirtualConnection* _virtualConnection;

	class TcpServer* _tcpServer;
};

} // End vnet.