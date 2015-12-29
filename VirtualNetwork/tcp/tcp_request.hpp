//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCP_REQUEST_HPP
#define TCP_REQUEST_HPP

#include "../Common.h"

#include <string>
#include <vector>
#include <map>

namespace vnet {

/// A request received from a client.

struct request
{
    //std::string controller;
    //std::string method;
	std::string key;

	uint32_t command;
	ArgumentList argumentList;

	template <typename Archive>
	friend Archive& operator << (Archive& ar, const request& r)
	{
		ar << r.command;
		ar << r.argumentList;
		return ar;
	}

	template <typename Archive>
	friend Archive& operator >> (Archive& ar, request& r)
	{
		r.argumentList.arguments.clear();
		ar >> r.command;
		ar >> r.argumentList;
		return ar;
	}

	const PTree* operator [] (const std::string& key) const
	{
		auto it = argumentList.arguments.find(key);
		return it != argumentList.arguments.end() ? (&it->second) : nullptr;
	}
};

} // namespace vnet.

#endif // TCP_REQUEST_HPP
