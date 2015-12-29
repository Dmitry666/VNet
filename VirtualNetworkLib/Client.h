#pragma once

#include "Common.h"
#include "Delegate.h"

namespace vnet {

/**
 * @brief Network client.
 */
class Client
{
public:
	/**
	 * @brief Client construct with central virtual network.
	 */
	VNET_API Client(class VirtualNetwork* virtualNetwork = nullptr);

	/**
	 * @brief destructor.
	 */
	VNET_API virtual ~Client();

	/**
	 * @brief Connect by address and port.
	 * @param address virtual or direct address.
	 * @param port remoute port.
	 *
	 * Try connect as direct socket. After connect with virtual network.
	 */
	VNET_API void Connect(const std::string& address, uint32 port);

	/**
	 * @brief Disconnect from server.
	 */
	VNET_API void Disconnect();

	/**
	 * @brief Send byte data.
 	 */
	VNET_API void Send(uint8* bytes, uint32 nbBytes);

	/**
	 * @brief Client already connected.
	 */
	VNET_API bool IsConnected() const;

public:
	Delegate<Client*> Connected;	//! Connect to server signal.
	Delegate<Client*> ConnectError; //! Connection error signal.
	Delegate<Client*> Disconnected; //! Client disconnected from server signal.
	Delegate<const uint8*, uint32> NewRequest; //! Server sended new data.

private:
	std::shared_ptr<class VirtualNetworkPrivate> _virtualNetworkPrivate;
	class VirtualConnection* _virtualConnection;

	class Socket* _socket;
};

} // End vnet.