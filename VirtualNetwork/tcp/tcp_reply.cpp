//
// reply.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "tcp_reply.hpp"
#include <string>

using namespace std;

namespace vnet {

std::vector<boost::asio::const_buffer> reply::to_buffers()
{
	size = 0;
	for (reply_block& content : contents)
		size += content.size;

    std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(&size, sizeof(uint32)));

    for (reply_block& content : contents)
    {		
        buffers.push_back(boost::asio::buffer(content.data));
    }
    return buffers;
}

} // namespace vnet.