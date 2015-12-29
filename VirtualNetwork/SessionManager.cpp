#include "SessionManager.h"
#include <algorithm>

#include <sqlite3.h>
#include <vector>
#include <tuple>
#include <iostream>


namespace vnet {

SessionManager::SessionManager()
{}

SessionManager::~SessionManager()
{}

SessionManager& SessionManager::Instance()
{
	static SessionManager sessionManager;
	return sessionManager;
}

void SessionManager::Init()
{

	sqlite3 *db = nullptr;
	char *zErrMsg = nullptr;
	int rc;

	// Open data bse.
	if (sqlite3_open("users.sqlite3", &db) == SQLITE_OK)
	{
		const char* sql_query = "CREATE TABLE IF NOT EXISTS keys (id INTEGER PRIMARY KEY AUTOINCREMENT, address TEXT, secret TEXT)";

		/* Execute SQL statement */
		rc = sqlite3_exec(db, sql_query, nullptr, nullptr, &zErrMsg);
		if (rc == SQLITE_OK)
		{
			std::cout << "Don't opened 'users.sqlite3' db." << std::endl;
		}
		else
		{
			std::cout << zErrMsg << std::endl;
			sqlite3_free(zErrMsg);
		}

		sqlite3_close(db);
	}
	//else
	{
		std::cout << "Don't opened 'users.sqlite3' db." << std::endl;
	}

}

SessionPtr SessionManager::Create()
{
	SessionKey key(rand());
	SessionPtr ptr = std::make_shared<Session>(key, key.ToString());

	_sessions.insert(std::make_pair(key, ptr));

	return ptr;
}

SessionPtr SessionManager::CreateBySecret(const std::string& secret)
{
	std::string address = FindAddressBySecret(secret);
	// Find address by secret.

	SessionKey key(rand());
	SessionPtr ptr = std::make_shared<Session>(key, address.empty() ? key.ToString() : address);

	_sessions.insert(std::make_pair(key, ptr));

	return ptr;
}

SessionPtr SessionManager::CreateByAddress(const std::string& address)
{
	// Find address by address.

	SessionKey key(rand());
	SessionPtr ptr = std::make_shared<Session>(key, address);

	_sessions.insert(std::make_pair(key, ptr));

	return ptr;
}

SessionPtr SessionManager::FindSession(const SessionKey& key)
{
	auto it = _sessions.find(key);
	return it != _sessions.end() ? it->second : nullptr;
}

SessionPtr SessionManager::FindSessionByAddress(const std::string& address)
{
	auto it = std::find_if(_sessions.begin(), _sessions.end(), [address](const std::pair<SessionKey, SessionPtr>& pair){
		return pair.second->Address() == address;
	});
	
	return it != _sessions.end() ? it->second : nullptr;
}

void SessionManager::Remove(SessionPtr session)
{
	auto it = std::find_if(_sessions.begin(), _sessions.end(), [session](const std::pair<SessionKey, SessionPtr>& pair){
		return pair.second == session;
	});

	if (it != _sessions.end())
	{
		_sessions.erase(it);
	}
}

int callbackKey(void *data, int argc, char **argv, char **azColName)
{
	//assert(argc == 2);
	std::vector<std::tuple<uint32_t, std::string>>& keys = *reinterpret_cast<std::vector<std::tuple<uint32_t, std::string>>*>(data);

	uint32_t id = atoi(argv[0]);
	std::string address = argv[1];

	keys.push_back(std::tuple<uint32_t, std::string>(id, address));

	return 0;
}

std::string SessionManager::FindAddressBySecret(const std::string& secret)
{
	std::string address;

#ifdef _DEBUG
	sqlite3 *db = nullptr;
	char *zErrMsg = nullptr;
	int rc;

	// Open data bse.
	const std::string& path = "users.sqlite3";
	if (sqlite3_open(path.c_str(), &db) == 0) //.c_str()
	{
		char sql_houses[4096];
		sprintf_s(sql_houses, "SELECT id, address, secret FROM keys WHERE secret='%s'", secret.c_str());

		/* Execute SQL statement */
		std::vector<std::tuple<uint32_t, std::string>> keys;
		rc = sqlite3_exec(db, sql_houses, callbackKey, static_cast<void*>(&keys), &zErrMsg);
		if (rc == SQLITE_OK)
		{
			if (keys.size() >= 1)
			{
				const std::tuple<uint32_t, std::string>& key = keys.front();
				address = std::get<1>(key);
			}
			else
			{
				// Nothing.
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
#endif

	return address;
}

} // End vnet.