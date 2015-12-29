#include "../VirtualNetworkLib/VN.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <thread>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


#include <boost/program_options.hpp>

namespace po = boost::program_options;

using namespace vnet;

Client* client = nullptr;

void connect(const std::string& address, const std::string& port)
{
	client->Connected = [address](Client* client){
		std::cout << "Client connected to: " << address << std::endl;
		client->NewRequest = [address](const uint8* data, uint32 nbBytes){
			MemoryReader in(MemoryBuffer(data, nbBytes));

			std::string text;
			in >> text;

			std::cout << "Client Message: " << text << std::endl;
		};

		client->Disconnected = [address](Client* client){
			std::cout << "Client disconnected for: " << address << std::endl;
		};
	};
	client->Connect(address, std::stoi(port));
}

void disconnect()
{
	client->Disconnect();
}

void send_message(const std::string& text)
{
	MemoryBuffer buffer;
	MemoryWriter out(buffer);

	out << text;

	client->Send(buffer.GetData(), buffer.GetSize());
}

void exec(const std::string& command)
{
	std::istringstream iss(command);

	std::vector<std::string> tokens;
	std::copy(std::istream_iterator<std::string>(iss),
		std::istream_iterator<std::string>(),
		std::back_inserter(tokens));
	
	if (tokens.empty())
		return;

	if ((tokens.size() == 2 || tokens.size() == 3) && tokens[0] == "connect")
	{
		connect(tokens[1], tokens.size() == 3 ? tokens[2] : "5001");
		return;
	}

	if (tokens.size() == 1 && tokens[0] == "disconnect")
	{
		disconnect();
		return;
	}

	if (tokens.size() >= 2 && tokens[0] == "message")
	{
		std::string text = tokens[1];
		for (int i = 2; i < tokens.size(); ++i)
			text += " " + tokens[i];

		send_message(text);
		return;
	}

	std::cout << "Unknown command: " << command;
}

int main(int argc, char *argv[])
{
	po::options_description desc("General options");
	std::string task_type;
	desc.add_options()
		("help,h", "Show help")
		("secret,s", po::value<std::string>(), "Secret key.")
		("static,t", po::value<std::string>(), "Static virtual address")
		("address,a", po::value<std::string>(), "Service address.")
		("port,p", po::value<uint32>(), "Service port.")
		;

	po::variables_map vm;
	po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
	po::store(parsed, vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		return 1;
	}

	// Show work directory.
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
	{
		return errno;
	}

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
	std::cout << "The current working directory is " << cCurrentPath << std::endl;

	// VirtualNetwork start.
	VirtualNetworkDesc vdesc{
		vm.count("address") ? vm["address"].as<std::string>() : "localhost",
		vm.count("port") ? vm["port"].as<uint32>() : 5000,
		vm.count("secret") ? vm["secret"].as<std::string>() : "",
		vm.count("static") ? vm["static"].as<std::string>() : ""
	};

	VirtualNetwork virtualNetwork(vdesc);
	Listerner listerner(5001, &virtualNetwork);
	client = new Client(&virtualNetwork);

	virtualNetwork.Authorizated = [&listerner](VirtualNetwork* virtualNetwork, const VirtualNetworkInfo& info){
		std::cout << "Connected to service. " << std::endl;
		std::cout << "Virtual Address. " << info.VirtualAddress << std::endl;

		listerner.Connected = [](Listerner* listerner, Socket* socket){
			std::cout << "Listerner: New connection. " << (socket->GetSocketType() == Socket::Virtual ? "Virtual" : "Direct") << std::endl;

			socket->NewRequest = [socket](const uint8* data, uint32 nbBytes){

				MemoryReader in(MemoryBuffer(data, nbBytes));

				std::string text;
				in >> text;

				std::cout << "Listerner Message: " << text << std::endl;

				MemoryBuffer buffer;
				MemoryWriter out(buffer);

				std::string responseText = "Hello my best friend.";
				out << responseText;

				socket->Send(buffer.GetData(), buffer.GetSize());
			};
		};
		listerner.Disconnected = [](Listerner* listerner, Socket* socket){
			std::cout << "Listerner: Remove connection." << std::endl;
		};

		listerner.Start();
		std::cout << "Listerner started on " << listerner.GetAddress() << ":" << listerner.GetPort() << std::endl;
		
	};
	virtualNetwork.Initialize();
	
	bool exit = false;

	virtualNetwork.Logouted = [&exit](VirtualNetwork* virtualNetwork){
		exit = true;
		std::cout << "Service logouted. Please key for exit." << std::endl;
	};

	do 
	{
		std::string line;
		printf("\n> ");
		std::getline(std::cin, line);

		if (!line.empty())
		{
			if (line == "quit")
			{
				client->Disconnect();
				listerner.Stop();
				virtualNetwork.Release();
			}
			else
				exec(line);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} 
	while (!exit);

	delete client;
	return 0;
}