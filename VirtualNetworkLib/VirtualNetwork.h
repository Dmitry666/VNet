#pragma once

#include "Common.h"
#include "Delegate.h"

namespace vnet {

struct VirtualClientInfo
{
	std::string LocalAddress;
	std::string VirtualAddress;
	std::string VirtualPort;
};

/**
 * @brief Virtual network info.
 */
struct VirtualNetworkInfo
{
	std::string VirtualAddress;
};

struct VirtualNetworkDesc
{
	std::string Address;
	uint32 Port;
	std::string Secret;
	std::string VirtualAddress;
};

/**
 * @brief Central virtual network.
 */
class VirtualNetwork
{
	friend class Listerner;
	friend class Client;

public:
	/**
	 * @brief Construct with description VN.
	 */
	VNET_API VirtualNetwork(const VirtualNetworkDesc& desc);
	/*VNET_API VirtualNetwork(const std::string& address, uint32 port, const std::string& secret = "");*/

	VNET_API virtual ~VirtualNetwork();

	/**
	 * @brief Start and connect to virtual netowk system.
	 */
	VNET_API void Initialize();

	/**
	 * @brief Stop and disconnect from virtual network system.
	 */
	VNET_API void Release();

	/**
	* @brief Check initialized system.
	*/
	VNET_API bool IsInitialized() const;

	VNET_API static VirtualNetwork& Instance(const std::string& name);
	VNET_API static VirtualNetwork& Instance(const std::string& name, const VirtualNetworkDesc& desc);

public:
	Delegate<VirtualNetwork*, const VirtualNetworkInfo&> Authorizated;
	Delegate<VirtualNetwork*> ConnectError;
	Delegate<VirtualNetwork*> Logouted;

private:
	std::shared_ptr<class VirtualNetworkPrivate> _virtualNetworkPrivate;
};

} // End vnet.