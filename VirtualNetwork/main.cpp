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


// Fir fork.
#ifndef _WIN32

#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <signal.h>

/*
 * checks if the string is purely an integer
 * we can do it with `strtol' also
 */
int check_if_number (char *str)
{
    int i;
    for (i=0; str[i] != '\0'; i++)
    {
        if (!isdigit (str[i]))
        {
            return 0;
        }
    }

    return 1;
}

#define MAX_BUF 1024
//#define PID_LIST_BLOCK 32

std::vector<int> pidof (char *pname)
{
    struct dirent *entry;
    char path[MAX_BUF], read_buf[MAX_BUF];

    DIR *dirp = opendir ("/proc/");
    if (dirp == NULL)
    {
        perror ("Fail");
        return std::vector<int>();
    }

    std::vector<int> pidlist; // = malloc (sizeof (int) * PID_LIST_BLOCK);

    while ((entry = readdir (dirp)) != nullptr)
    {
        if (check_if_number (entry->d_name))
        {
            strcpy (path, "/proc/");
            strcat (path, entry->d_name);
            strcat (path, "/comm");

            /* A file may not exist, it may have been removed.
            * dut to termination of the process. Actually we need to
            * make sure the error is actually file does not exist to
            * be accurate.
            */
            FILE *fp = fopen (path, "r");
            if (fp != NULL)
            {
                fscanf (fp, "%s", read_buf);
                if (strcmp (read_buf, pname) == 0)
                {
                    /* add to list and expand list if needed */
                    pidlist.push_back(atoi (entry->d_name));
                }

                fclose (fp);
            }
        }
    }


    closedir (dirp);
    //pidlist[pidlist_index] = -1; /* indicates end of list */
    return pidlist;
}

int startChild()
{
    int pid = fork();
    if(pid == 0)
    {
        return -1;
    }

    printf("Start child process (PID: %d)\n", pid);
    return 0;
}

int stopChild(char *pname)
{
    printf("Stop child process '%s'", pname);
    std::vector<int> pids = pidof(pname);

    for(int pid : pids)
    {
        printf("Kill %s - %d\n", pname, pid);
        kill(pid, 3);
    }

    return 0;
}
#endif


// Crush.
#ifndef _WIN32
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


void exceptionHandler(int sig)
{
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif

// WITH_NETWORKAPI

int main(int argc, char* argv[])
{
#ifndef _WIN32
    signal(SIGSEGV, exceptionHandler);
#endif

	po::options_description desc("General options");
	std::string task_type;
	desc.add_options()
		("help,h", "Show help")
#ifndef _WIN32
        ("start,s", "Start as child fork.")
        ("stop,b", "Stop child fork.")
#endif
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

#ifndef _WIN32
    if (vm.count("start"))
    {
        int pid = startChild();
        if(pid != -1)
            return 0;
        return -1;
    }

    if (vm.count("stop"))
    {
        return stopChild(argv[0]);
    }
#endif

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

	try
	{	
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
	catch (std::exception e)
	{
		cout << e.what() << endl;
	}
}
