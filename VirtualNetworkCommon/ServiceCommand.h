#pragma once

#include "Common.h"
#include "Archive.h"
#include "PTree.h"
#include <map>


namespace vnet{

/**
 * @brief Request command list.
 */
enum class RequestCommands
{
	Authorize,
	Logout,
	NewAddress,
	CheckStatus,
	ReadClients,

	Ping,

	ClientConnect,
	ClientDisconnect,
	SendCommand,
	ReadCommand,
	ReadCommands
};


/**
 * @brief Reponse command list.
 */
enum class ResponseCommands
{
	Authorizated,
	Logouted,
	NewAddress,
	Status,
	Clients,

	Pong,

	ClientNotFound,
	ClientConnect,
	ClientDisconnect,

	ClientConnected,
	ClientDisconnected,
	CommandSended,

	SendCommand,
	NoCommands,
	Command,
	Commands,

	AccessDenied
};

/**
 * @brief Argument list struct.
 */
struct ArgumentList
{
	//std::map<std::string, Variant> arguments;
	std::map<std::string, PTree> arguments;

	friend Archive& operator << (Archive& archive, const ArgumentList& list)
	{
		uint32 nbElements = list.arguments.size();
		archive << nbElements;

		for (auto& pair : list.arguments)
		{
			archive << pair.first << pair.second;
		}

		return archive;
	}

	friend Archive& operator >> (Archive& archive, ArgumentList& list)
	{
		uint32 nbElements;
		archive >> nbElements;

		for (uint32 i = 0; i < nbElements; ++i)
		{
			std::string name;
			PTree argument;

			archive >> name >> argument;
			list.arguments.insert(std::make_pair(name, argument));
		}

		return archive;
	}
};

/**
* @brief Service command struct.
*/
struct ServiceCommand
{
	uint32 CommandKey;
	ArgumentList argumentList;
	//std::map<std::string, PTree> argumentList;

	ServiceCommand()
	{}

	ServiceCommand(uint32 commandKey)
		: CommandKey(commandKey)
	{}

	ServiceCommand(RequestCommands requestCommands)
		: CommandKey(static_cast<uint32>(requestCommands))
	{}

	ServiceCommand(ResponseCommands responseCommands)
		: CommandKey(static_cast<uint32>(responseCommands))
	{}

	void Push(const std::string& key, const Variant& argument)
	{
		argumentList.arguments.insert(std::make_pair(key, PTree(argument)));
	}

	void Push(const std::string& key, const PTree& node)
	{
		argumentList.arguments.insert(std::make_pair(key, node));
	}

	const PTree* operator [] (const std::string& key) const
	{
		auto it = argumentList.arguments.find(key);
		return it != argumentList.arguments.end() ? (&it->second) : nullptr;
	}

	/*
	const Variant* operator [] (const std::string& key) const
	{
	auto it = argumentList.arguments.find(key);
	return it != argumentList.arguments.end() ? (&it->second->Data) : nullptr;
	}
	*/

	friend Archive& operator << (Archive& archive, const ServiceCommand& cmd)
	{
		archive << cmd.CommandKey;
		archive << cmd.argumentList;

		return archive;
	}

	friend Archive& operator >> (Archive& archive, ServiceCommand& cmd)
	{
		archive >> cmd.CommandKey;
		archive >> cmd.argumentList;

		return archive;
	}
};

} // End vnet.