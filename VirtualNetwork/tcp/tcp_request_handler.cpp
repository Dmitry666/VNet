//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "tcp_request_handler.hpp"
#include "tcp_reply.hpp"
#include "tcp_request.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "../SessionManager.h"

using namespace std;

namespace vnet {

request_handler::request_handler(SessionManager& manager)
	: manager_(manager)
{}

void request_handler::handle_new_connection()
{}

void request_handler::handle_remove_connection(const std::string& key)
{
	if (!key.empty())
	{
		SessionPtr session = manager_.FindSession(key);
		if (session != nullptr)
		{
			manager_.Remove(session);
		}
	}
}

void request_handler::handle_request(const request& req, reply& rep)
{
	RequestCommands cmd = static_cast<RequestCommands>(req.command);

	static std::map<RequestCommands, std::function<void(request_handler&, const request&, reply&)>> commands = {
		std::make_pair(RequestCommands::Authorize, std::mem_fn(&request_handler::handle_login)),
		std::make_pair(RequestCommands::Logout, std::mem_fn(&request_handler::handle_logout)),
		std::make_pair(RequestCommands::NewAddress, std::mem_fn(&request_handler::handle_new_address)),
		std::make_pair(RequestCommands::ReadClients, std::mem_fn(&request_handler::handle_read_clients)),
		std::make_pair(RequestCommands::Ping, std::mem_fn(&request_handler::handle_ping)),

		std::make_pair(RequestCommands::ClientConnect, std::mem_fn(&request_handler::handle_connect_client)),
		std::make_pair(RequestCommands::ClientDisconnect, std::mem_fn(&request_handler::handle_disconnect_client)),
		std::make_pair(RequestCommands::SendCommand, std::mem_fn(&request_handler::handle_send_command)),
		std::make_pair(RequestCommands::ReadCommand, std::mem_fn(&request_handler::handle_read_command)),
		std::make_pair(RequestCommands::ReadCommands, std::mem_fn(&request_handler::handle_read_commands)),	
	};

	auto it = commands.find(cmd);
	if (it != commands.end())
	{
		it->second(*this, req, rep);
	}
	else
	{
		std::cout << "handle_request command not found." << std::endl;
	}
}

//
void request_handler::handle_login(const request& req, reply& rep)
{
#ifdef _DEBUG
	//std::cout << "handle_login" << std::endl;
#endif

	const PTree* virtualAddressValue = req["virtualAddress"];
	const PTree* localAddressValue = req["localAddress"];
	const PTree* secretValue = req["secret"];

	SessionPtr session = 
		secretValue != nullptr ? manager_.CreateBySecret(secretValue->Data.AsString()) : 
		virtualAddressValue != nullptr ? manager_.CreateByAddress(virtualAddressValue->Data.AsString()) : manager_.Create();
	const std::string& address = session->Address();

#ifdef _DEBUG
	std::cout << session->Address() << ": handle_login: " << std::endl;
#endif

	// Response.
	ServiceCommand cmd(ResponseCommands::Authorizated);
	cmd.Push("address", address);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.key = session->Key().ToString();
	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_logout(const request& req, reply& rep)
{
#ifdef _DEBUG
	std::cout << req.key << ": handle_logout: " << std::endl;
#endif

	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	manager_.Remove(session);

	// Response.
	ServiceCommand cmd(ResponseCommands::Logouted);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.key.clear();
	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_new_address(const request& req, reply& rep)
{
#ifdef _DEBUG
	std::cout << req.key << ": handle_new_address" << std::endl;
#endif

	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	const std::string& address = session->Address();

	// Response.
	ServiceCommand cmd(ResponseCommands::NewAddress);
	cmd.Push("address", Variant(address));

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_read_clients(const request& req, reply& rep)
{
#ifdef _DEBUG
	std::cout << req.key << ": handle_read_clients" << std::endl;
#endif

	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	const std::map<SessionKey, SessionPtr>& sessions = manager_.GetSessions();

	PTree nodeClients;
	for (auto pair : sessions)
	{
		SessionPtr session = pair.second;
		nodeClients.Put("", session->Address());
	}
	
	// Response.
	ServiceCommand cmd(ResponseCommands::Clients);
	cmd.Push("clients", nodeClients);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_ping(const request& req, reply& rep)
{
	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	// Response.
	ServiceCommand cmd(ResponseCommands::Pong);
	// cmd.Push("success", address);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_connect_client(const request& req, reply& rep)
{
#ifdef _DEBUG
	std::cout << req.key << ": handle_connect_client" << std::endl;
#endif

	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	const PTree* addressValue = req["address"];
	const PTree* portValue = req["port"];
	const PTree* responsePortValue = req["responsePort"];
	
	if (
		(addressValue == nullptr || !addressValue->Data.IsString()) ||
		(portValue == nullptr || !portValue->Data.IsString()) ||
		(responsePortValue == nullptr || !responsePortValue->Data.IsString())
		)
	{
		rep = reply::client_not_found(req.key);
		return;
	}
 
	SessionPtr otherSession = manager_.FindSessionByAddress(addressValue->Data.AsString());
	if (otherSession == nullptr)
	{
		MemoryBuffer buffer;
		ServiceCommand cmd(ResponseCommands::ClientNotFound);
		cmd.Push("address", addressValue->Data.AsString());
		cmd.Push("port", portValue->Data.AsString());
		cmd.Push("responsePort", responsePortValue->Data.AsString());
	
		MemoryWriter out(buffer);
		out << cmd;

		rep.append(buffer.GetData(), buffer.GetSize());
		return;
	}

	// Send to other client.
	{
		MemoryBuffer buffer;
		ServiceCommand cmd(ResponseCommands::ClientConnect);
		cmd.Push("address", session->Address());
		cmd.Push("port", portValue->Data.AsString());
		cmd.Push("responsePort", responsePortValue->Data.AsString());
		
		MemoryWriter out(buffer);
		out << cmd;

		otherSession->PushPackage(buffer.GetData(), buffer.GetSize());
	}

	// Response Success.
	MemoryBuffer buffer;
	ServiceCommand cmd(ResponseCommands::ClientConnected); // TODO. Bad code.
	cmd.Push("port", responsePortValue->Data.AsString());
	MemoryWriter out(buffer);
	out << cmd;

	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_disconnect_client(const request& req, reply& rep)
{
#ifdef _DEBUG
	std::cout << req.key << ": handle_disconnect_client" << std::endl;
#endif

	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	const PTree* addressValue = req["address"];
	const PTree* portValue = req["port"];
	const PTree* responsePortValue = req["responsePort"];

	if (
		(addressValue == nullptr || !addressValue->Data.IsString()) ||
		(portValue == nullptr || !portValue->Data.IsString()) ||
		(responsePortValue == nullptr || !responsePortValue->Data.IsString())
		)
	{
		rep = reply::client_not_found(req.key);
		return;
	}

	SessionPtr otherSession = manager_.FindSessionByAddress(addressValue->Data.AsString());
	if (otherSession == nullptr)
	{
		MemoryBuffer buffer;
		ServiceCommand cmd(ResponseCommands::ClientNotFound);
		cmd.Push("address", addressValue->Data.AsString());
		cmd.Push("port", portValue->Data.AsString());
		cmd.Push("responsePort", responsePortValue->Data.AsString());

		MemoryWriter out(buffer);
		out << cmd;

		rep.append(buffer.GetData(), buffer.GetSize());
		//rep = reply::client_not_found(req.key, responsePortValue->Data.AsString());
		return;
	}

	// Send to other client.
	{
		MemoryBuffer buffer;
		ServiceCommand cmd(ResponseCommands::ClientDisconnect);
		cmd.Push("address", session->Address());
		cmd.Push("port", portValue->Data.AsString());
		cmd.Push("responsePort", responsePortValue->Data.AsString());
		MemoryWriter out(buffer);
		out << cmd;

		otherSession->PushPackage(buffer.GetData(), buffer.GetSize());
	}

	// Response.
	ServiceCommand cmd(ResponseCommands::ClientDisconnected); // TODO. Bad code.
	cmd.Push("port", responsePortValue->Data.AsString());

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_send_command(const request& req, reply& rep)
{
#ifdef _DEBUG
	std::cout << req.key << ": handle_send_command" << std::endl;
#endif

	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	const PTree* addressValue = req["address"];
	const PTree* portValue = req["port"];
	const PTree* responsePortValue = req["responsePort"];
	const PTree* dataValue = req["data"];

	if (
		(addressValue == nullptr || !addressValue->Data.IsString()) ||
		(portValue == nullptr || !portValue->Data.IsString()) ||
		(dataValue == nullptr || !dataValue->Data.IsBytes()) ||
		(responsePortValue == nullptr || !responsePortValue->Data.IsString())
		)
	{
		rep = reply::client_not_found(req.key);
		return;
	}

	SessionPtr otherSession = manager_.FindSessionByAddress(addressValue->Data.AsString());
	if (otherSession == nullptr)
	{
		MemoryBuffer buffer;
		ServiceCommand cmd(ResponseCommands::ClientNotFound);
		cmd.Push("address", addressValue->Data.AsString());
		cmd.Push("port", portValue->Data.AsString());
		cmd.Push("responsePort", responsePortValue->Data.AsString());

		MemoryWriter out(buffer);
		out << cmd;

		rep.append(buffer.GetData(), buffer.GetSize());
		//rep = reply::client_not_found(req.key, responsePortValue->Data.AsString());
		return;
	}

	// Send to other client.
	{
		const std::vector<uint8> data = dataValue->Data.AsBytes();

		MemoryBuffer buffer;
		ServiceCommand cmd(ResponseCommands::Command);
		cmd.Push("address", session->Address());
		cmd.Push("port", portValue->Data.AsString());
		cmd.Push("responsePort", responsePortValue->Data.AsString());
		cmd.Push("data", Variant(data.data(), data.size()));
		MemoryWriter out(buffer);
		out << cmd;

		otherSession->PushPackage(buffer.GetData(), buffer.GetSize());
	}

	// Response.
	ServiceCommand cmd(ResponseCommands::CommandSended); // TODO. Bad command.
	cmd.Push("port", responsePortValue->Data.AsString());

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.append(buffer.GetData(), buffer.GetSize());
}

void request_handler::handle_read_command(const request& req, reply& rep)
{
	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPackage* sessionPackage = session->FrontPackage();

	if (sessionPackage == nullptr)
	{
		ServiceCommand cmd(ResponseCommands::NoCommands);
		MemoryBuffer buffer;
		MemoryWriter out(buffer);
		out << cmd;

		rep.append(buffer.GetData(), buffer.GetSize());
		return;
	}
	
#ifdef _DEBUG
	std::cout << "resened command to " << session->Address() << std::endl;
#endif

	// Response.
	rep.append(sessionPackage->data, sessionPackage->size);
	session->PopPackage();

	/*
		ServiceCommand cmd(ResponseCommands::Command);

		cmd.Push("data", Variant(sessionPackage->data, sessionPackage->size));
		session->PopPackage();

		MemoryBuffer buffer;
		MemoryWriter out(buffer);

		out << cmd;

		rep.append(buffer.GetData(), buffer.GetSize());
	*/	
}

void request_handler::handle_read_commands(const request& req, reply& rep)
{
	if (req.key.empty())
	{
#ifdef _DEBUG
		std::cout << "session not valid." << std::endl;
#endif
		rep = reply::access_denied(req.key);
		return;
	}

	SessionPtr session = manager_.FindSession(req.key);
	if (session == nullptr) // Client don't logined.
	{
		rep = reply::access_denied(req.key);
		return;
	}

	//
	SessionPackage* sessionPackage = session->FrontPackage();
	if (sessionPackage == nullptr)
	{
		ServiceCommand cmd(ResponseCommands::NoCommands);
		MemoryBuffer buffer;
		MemoryWriter out(buffer);
		out << cmd;

		rep.append(buffer.GetData(), buffer.GetSize());
		return;
	}

	//!
	int32 nbPackages = 0;
	while (sessionPackage)
	{
		//commandsNode.Put("", Variant(sessionPackage->data, sessionPackage->size));

		rep.append(sessionPackage->data, sessionPackage->size);
		session->PopPackage();

		sessionPackage = session->FrontPackage();

		++nbPackages;
	}
	
#ifdef _DEBUG
	std::cout << "resened (" << nbPackages << ") command to " << session->Address() << std::endl;
#endif

	/*
	cmd.Push("commands", commandsNode);

	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << cmd;

	rep.append(buffer.GetData(), buffer.GetSize());

	*/
}

} // namespace vnet.