#pragma once

#include "Common.h"


namespace vnet {

/**
 * @brief Tcp command.
 */
struct TcpCommand
{
	uint8* Data;
	uint32 Size;

	TcpCommand()
		: Data(nullptr)
		, Size(0)
	{}

	TcpCommand(const uint8* data, uint32 nbBytes)
		: Size(nbBytes)
	{
		Data = reinterpret_cast<uint8*>(malloc(nbBytes)); // TODO. Allocator.
		memcpy(Data, data, nbBytes);
	}

	TcpCommand(const TcpCommand& tcpCommand)
		: Size(tcpCommand.Size)
	{
		Data = reinterpret_cast<uint8*>(malloc(Size)); // TODO. Allocator.
		memcpy(Data, tcpCommand.Data, Size);
	}

	TcpCommand(TcpCommand&& tcpCommand)
		: Size(tcpCommand.Size)
	{
		Data = tcpCommand.Data;
		tcpCommand.Data = nullptr;
	}

	~TcpCommand()
	{
		free(Data);
	}
};

} // End vnet.