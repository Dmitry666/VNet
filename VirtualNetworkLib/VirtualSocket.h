#pragma once

#include "Socket.h"


namespace vnet {

/**
 * @brief Socket class for virtual network server.
 */
class VirtualSocket : public Socket
{
public:
	VirtualSocket(class VirtualConnection* conenction);
	VirtualSocket(class VirtualConnection* conenction, const struct VirtualClientInfo& vci);
	virtual ~VirtualSocket();
	
	VirtualSocket(const VirtualSocket&) = delete;
	VirtualSocket& operator = (const VirtualSocket&) = delete;

	virtual void Connect(const std::string& address, int32 port) override;
	virtual void Disconnect() override;

	virtual void Send(const uint8* bytes, int32 nbBytes) override;
	virtual void Read(uint8* bytes, int32 nbBytes) override;

	virtual bool IsValid() const override
	{
		return !_address.empty();
	}

	virtual SocketType GetSocketType() const override { return Virtual; }

	virtual bool IsConnected() const override;

	const std::string& GetAddress() const { return _address; }

public:
	Delegate<VirtualSocket*> Connected;
	Delegate<VirtualSocket*> ConnectError;
	Delegate<VirtualSocket*> Disconnected;

public:
	void OnNewRequest(const VirtualClientInfo&, const uint8* bytes, int32 nbBytes);

private:
	//class VirtualNetworkPrivate* _vnp;
	class VirtualConnection* _virtualConnection;

	std::string _address;
	std::string _port;
};

} // End vnet.