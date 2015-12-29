// Author: Oznabikhin Dmitry
// Email: gamexgroup@gmail.ru
//
// Copyright (c) UTS. All rights reserved.
#pragma once

#include <memory>
#include <map>
#include <deque>
#include <string>

namespace vnet {

/**
 * @brief Session key class.
 */
struct SessionKey
{
	int32_t value0;

	SessionKey()
		: value0(0)
	{}

	SessionKey(int32_t v0)
		: value0(v0)
	{}

	SessionKey(const std::string& key)
	{
		value0 = std::stoi(key);
	}
	
	SessionKey(const SessionKey& sk)
		: value0(sk.value0)
	{}

	std::string ToString() const
	{
		return std::to_string(value0);
	}

	friend bool operator == (const SessionKey& a, const SessionKey& b)
	{
		return a.value0 == b.value0;
	}

	friend bool operator != (const SessionKey& a, const SessionKey& b)
	{
		return a.value0 != b.value0;
	}

	friend bool operator < (const SessionKey& a, const SessionKey& b)
	{
		return a.value0 < b.value0;
	}

	friend bool operator >(const SessionKey& a, const SessionKey& b)
	{
		return a.value0 > b.value0;
	}
};

/**
 * @brief Session package class.
 */
struct SessionPackage
{
	/*
	SessionPackage(const std::string& addr, const uint8_t* bytes, int32_t nb_bytes)
		: address(addr)
		, size(nb_bytes)
	{
		data = reinterpret_cast<uint8_t*>(malloc(nb_bytes)); // TODO. Allocator.
		memcpy(data, bytes, nb_bytes);
	}
	*/

	SessionPackage(const uint8_t* bytes, int32_t nb_bytes)
		: size(nb_bytes)
	{
		data = reinterpret_cast<uint8_t*>(malloc(nb_bytes)); // TODO. Allocator.
		memcpy(data, bytes, nb_bytes);
	}

	~SessionPackage()
	{
		free(data);
	}

	//std::string address;
	uint8_t* data;
	uint32_t size;
};

/**
 * @brief Session class.
 */
class Session : public std::enable_shared_from_this<Session>
{
public:
	Session()
	{}

	Session(SessionKey key, const std::string& address)
		: _key(key)
		, _address(address)
	{}

	
	~Session()
	{
		for (SessionPackage* package : _packages)
			delete package;
		_packages.clear();
	}

	const SessionKey& Key() const { return _key; }
	const std::string& Address() const { return _address; }

	void PushPackage(SessionPackage* package)
	{
		_packages.push_back(package);
	}

	/*
	void PushPackage(const std::string& address, uint8_t* bytes, int32_t nb_bytes)
	{
		_packages.push_back(new SessionPackage(address, bytes, nb_bytes));
	}
	*/

	void PushPackage(const uint8_t* bytes, int32_t nb_bytes)
	{
		_packages.push_back(new SessionPackage(bytes, nb_bytes));
	}

	SessionPackage* FrontPackage()
	{
		if (_packages.empty())
			return nullptr;

		return _packages.front();
	}

	void PopPackage()
	{
		_packages.pop_front();
	}

protected:
	SessionKey _key;
	std::string _address;
	std::deque<SessionPackage*> _packages;
};

typedef std::shared_ptr<Session> SessionPtr;

/**
 * @brief Thread session from client.
 */
class SessionManager
{
private:
	SessionManager();
	~SessionManager();

public:
	void Init();

	SessionPtr Create();
	SessionPtr CreateBySecret(const std::string& secret);
	SessionPtr CreateByAddress(const std::string& address);

	SessionPtr FindSession(const SessionKey& key);
	SessionPtr FindSession(const std::string& key)
	{
		return FindSession(SessionKey(key));
	}

	SessionPtr FindSessionByAddress(const std::string& address);

	void Remove(SessionPtr session);

	const std::map<SessionKey, SessionPtr>& GetSessions() const { return _sessions; }

	//!
	static SessionManager& Instance();

private:
	std::string FindAddressBySecret(const std::string& secret);

public:
	std::map<SessionKey, SessionPtr> _sessions;
	
};


} // namespace vnet.