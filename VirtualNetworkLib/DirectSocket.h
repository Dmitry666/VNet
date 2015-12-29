#pragma once

#include "Socket.h"


namespace vnet {

/**
 * @brief Socket class for direct connection.
 */
class DirectSocket : public Socket
{
public:	
	DirectSocket();
	DirectSocket(class TcpConnection* tcpConnection);
	virtual ~DirectSocket();

	DirectSocket(const DirectSocket&) = delete;
	DirectSocket& operator = (const DirectSocket&) = delete;

	virtual void Connect(const std::string& address, int32 port) override;
	virtual void Disconnect() override;

	virtual void Send(const uint8* bytes, int32 nbBytes) override;
	virtual void Read(uint8* bytes, int32 nbBytes) override;

	virtual bool IsValid() const override
	{
		return true;
	}

	virtual SocketType GetSocketType() const override { return Direct; }

	virtual bool IsConnected() const override;

	TcpConnection* GetConnection() const { return _tcpConnection; }

public:
	Delegate<DirectSocket*> Connected;
	Delegate<DirectSocket*> ConnectError;
	Delegate<DirectSocket*> Disconnected;

private:
	void OnNewRequest(const uint8* bytes, int32 nbBytes);

private:
	class TcpClient* _tcpClient;
	class TcpConnection* _tcpConnection;
};

} // End vnet.