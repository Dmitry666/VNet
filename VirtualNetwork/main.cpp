#include "tcp/tcp_server.hpp"
#include "SessionManager.h"

#include <iostream>

#ifdef WITH_NETWORKAPI
#include <httpservice.h>
#endif

using namespace vnet;
#ifdef WITH_NETWORKAPI
using namespace openrc;
#endif
using namespace std;

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

#ifdef WITH_NETWORKAPI
#pragma comment(lib,"HTTPCommandService.lib")
//#pragma comment(lib,"jsoncpp_mt.lib")
#endif

// WITH_NETWORKAPI

int main(int argc, char* argv[])
{
	po::options_description desc("General options");
	std::string task_type;
	desc.add_options()
		("help,h", "Show help")
		("port,p", po::value<uint32_t>(), "Port for local server.")
		("webport,w", po::value<uint32_t>(), "Port for web server.")
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

	//!
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
	{
		return errno;
	}

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
	std::cout << "The current working directory is " << cCurrentPath << std::endl;


	std::string address = "0.0.0.0";
	int32_t port = vm.count("port") ? vm["port"].as<uint32_t>() : 5000;

	std::cout << "Start service: (" << address << ":" << port << ")" << std::endl;

	SessionManager& sessionManager = SessionManager::Instance();
	sessionManager.Init();

	request_handler handler(sessionManager);
	tcp_server server(address, std::to_string(port), handler);


#ifdef WITH_NETWORKAPI
	HttpServiceArguments arguments;
	arguments.Push("thread_pool_size", "1");

	HttpService service(arguments);

	string orcAddress = "0.0.0.0";
	int32_t orcPort = vm.count("webport") ? vm["webport"].as<uint32_t>() : 20202;

	cout << "Start HTTP Service: " << orcAddress << ":" << orcPort << endl;
	service.Start(orcAddress, std::to_string(orcPort));
#endif
	
	server.start();


	bool bExit = false;
	do
	{
		std::string command;
		getline(std::cin, command);
		transform(command.begin(), command.end(), command.begin(), ::tolower);

		if (command == "stop" || command == "exit")
			bExit = true;
	} 
	while (!bExit);

#ifdef WITH_NETWORKAPI
	service.Stop();
	service.Join();
#endif

	server.stop();
}