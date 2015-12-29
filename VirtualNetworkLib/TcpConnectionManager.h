#pragma once

#include <set>
#include "TcpConnection.h" 

namespace vnet {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class TcpConnectionManager
{
public:
	TcpConnectionManager(const TcpConnectionManager&) = delete;
	TcpConnectionManager& operator=(const TcpConnectionManager&) = delete;

    /// Construct a connection manager.
	TcpConnectionManager();

    /// Add the specified connection to the manager and start it.
	void Start(TcpConnectionPtr c);

    /// Stop the specified connection.
	void Stop(TcpConnectionPtr c);

    /// Stop all connections.
    void StopAll();

private:
    /// The managed connections.
	std::set<TcpConnectionPtr> connections_;
};

} // namespace vnet.
