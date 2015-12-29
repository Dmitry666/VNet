//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "TcpConnectionManager.h"

namespace vnet {

TcpConnectionManager::TcpConnectionManager()
{}

void TcpConnectionManager::Start(TcpConnectionPtr c)
{
    connections_.insert(c);
    c->Start();
}

void TcpConnectionManager::Stop(TcpConnectionPtr c)
{
    connections_.erase(c);
    c->Stop();
}

void TcpConnectionManager::StopAll()
{
    for (auto c: connections_)
        c->Stop();
    connections_.clear();
}

} // namespace vnet.