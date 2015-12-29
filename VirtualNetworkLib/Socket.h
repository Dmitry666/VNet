#pragma once

#include "Common.h"
#include "Delegate.h"

namespace vnet {



/**
 * @brief Network socket.
 */
class Socket
{
public:
	enum SocketType
	{
		None,
		Direct,
		Virtual
	};

public:
	/**
	 * @brief Default constructor.
	 */
	Socket()
	{}

	/**
	 * @brief Default destructor.
	 */
	virtual ~Socket()
	{}

	Socket(const Socket&) = delete;
	Socket& operator = (const Socket&) = delete;

	// Abstract methods.

	virtual void Connect(const std::string& address, int32 port) = 0;
	virtual void Disconnect() = 0;

	virtual void Send(const uint8* bytes, int32 nbBytes) = 0;

	/**
	 * TODO. Read don't work. User NewRequest signal.
	 */
	virtual void Read(uint8* bytes, int32 nbBytes) = 0;

	virtual bool IsValid() const = 0;

	virtual bool IsConnected() const = 0;

	virtual SocketType GetSocketType() const { return None; }

public:
	Delegate<const uint8*, uint32> NewRequest;
};

} // End vnet.