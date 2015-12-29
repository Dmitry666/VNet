//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCP_REPLY_HPP
#define TCP_REPLY_HPP

#include "../Common.h"
#include <vector>

#ifndef _MSC_VER
// save diagnostic state
#pragma GCC diagnostic push
// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wall"
//#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#define BOOST_CONFIG_WARNING_DISABLE_HPP
#pragma GCC system_header
#endif

#include <boost/asio.hpp>

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

namespace vnet {

/// A reply to be sent to a client.
struct reply_block
{
	uint32 size;
	std::vector<uint8_t> data;

	reply_block(const uint8* data, uint32 nbBytes)
		: size(nbBytes)
		, data(data, data + nbBytes)
	{}

	reply_block(const reply_block& rb)
		: size(rb.size)
		, data(rb.data)
	{}

	reply_block(reply_block&& rb)
		: size(rb.size)
		, data(std::move(rb.data))
	{}
};


struct reply
{
	std::string key;

	uint32 size;
	std::vector<reply_block> contents;

	void reset()
	{
		contents.clear();
	}

    std::vector<boost::asio::const_buffer> to_buffers();

	void append(const uint8* data, uint32 nbBytes)
	{
		contents.push_back(reply_block(data, nbBytes));	
	}

	static reply access_denied(const std::string& key)
	{
		reply rep;

		ServiceCommand cmd(ResponseCommands::AccessDenied);
		MemoryBuffer buffer;
		MemoryWriter out(buffer);
		out << cmd;

		rep.key = key;
		rep.append(buffer.GetData(), buffer.GetSize());
		return rep;
	}
	
	static reply client_not_found(const std::string& key)
	{
		reply rep;

		ServiceCommand cmd(ResponseCommands::ClientNotFound);
		MemoryBuffer buffer;
		MemoryWriter out(buffer);
		out << cmd;

		rep.key = key;
		rep.append(buffer.GetData(), buffer.GetSize());
		return rep;
	}
};

} // namespace vnet.

#endif // TCP_REPLY_HPP
