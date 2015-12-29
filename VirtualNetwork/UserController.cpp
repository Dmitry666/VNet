#include "UserController.h"

#ifdef WITH_NETWORKAPI
#include <json/reader.h>

#include <sstream>
#include <sqlite3.h>

using namespace std;

namespace openrc {

CONTROLLER_REGISTERIMPL(UserController)
bool UserController::Construct()
{
    return true;
}

CONTROLLER_ACTIONIMPL(UserController, AddUser, "AddUser", "Add new user.")
void UserController::AddUser(SessionWeak sessionWeak, const ControllerArguments& arguments, ControllerOutput& outContents)
{
	auto session = sessionWeak.lock();

	std::string address = arguments["address"];
	std::string secret = arguments["secret"];

	//
	sqlite3 *db = nullptr;
	char *zErrMsg = nullptr;
	int rc;

	// Open data bse.
	const std::string& path = "users.sqlite3";
	if (sqlite3_open(path.c_str(), &db) == 0)
	{
		char sql_query[4096];
		sprintf_s(sql_query, "INSERT INTO keys (address, secret)  VALUES('%s', '%s')", address.c_str(), secret.c_str());

		/* Execute SQL statement */
		std::vector<std::tuple<uint32_t, std::string>> keys;
		rc = sqlite3_exec(db, sql_query, nullptr, nullptr, &zErrMsg);
		if (rc == SQLITE_OK)
		{
			std::cout << "Added new key: " << address << " " << secret << std::endl;
		}
		else
		{
			std::cout << zErrMsg << std::endl;
			sqlite3_free(zErrMsg);
		}

		sqlite3_close(db);
	}
	else
	{
		std::cout << "Don't opened 'users.sqlite3' db." << std::endl;
	}
	//

	outContents.Append("OK");
}


CONTROLLER_ACTIONIMPL(UserController, RemoveUser, "RemoveUser", "Remove user.")
void UserController::RemoveUser(SessionWeak sessionWeak, const ControllerArguments& arguments, ControllerOutput& outContents)
{
	auto session = sessionWeak.lock();
	outContents.Append("OK");
}


int callbackKey(void *data, int argc, char **argv, char **azColName)
{
	//assert(argc == 2);
	std::vector<std::tuple<uint32_t, std::string, std::string>>& keys 
		= *reinterpret_cast<std::vector<std::tuple<uint32_t, std::string, std::string>>*>(data);

	uint32_t id = atoi(argv[0]);
	std::string address = argv[1];
	std::string secret = argv[2];

	keys.push_back(std::tuple<uint32_t, std::string, std::string>(id, address, secret));

	return 0;
}


CONTROLLER_ACTIONIMPL(UserController, GetUsers, "GetUsers", "Get users.")
void UserController::GetUsers(SessionWeak sessionWeak, const ControllerArguments& arguments, ControllerOutput& outContents)
{
	auto session = sessionWeak.lock();

	//
	sqlite3 *db = nullptr;
	char *zErrMsg = nullptr;
	int rc;


	// Open data bse.
	const std::string& path = "users.sqlite3";
	if (sqlite3_open(path.c_str(), &db) == 0) //.c_str()
	{
		char sql_houses[4096];
		sprintf_s(sql_houses, "SELECT id, address, secret FROM keys");

		/* Execute SQL statement */
		std::vector<std::tuple<uint32_t, std::string, std::string>> keys;
		rc = sqlite3_exec(db, sql_houses, callbackKey, static_cast<void*>(&keys), &zErrMsg);
		if (rc == SQLITE_OK)
		{
			for (const std::tuple<uint32_t, std::string, std::string>& key : keys)
			{
				std::string text = std::to_string(std::get<0>(key)) + ": " + std::get<1>(key) +" - " + std::get<2>(key) + "\n";
				outContents.Append(text);
			}
		}
		else
		{
			std::cout << zErrMsg << std::endl;
			sqlite3_free(zErrMsg);
		}

		sqlite3_close(db);
	}
	else
	{
		std::cout << "Don't opened 'users.sqlite3' db." << std::endl;
	}
	//

	outContents.Append("OK");
}

} // End openrc.
#endif