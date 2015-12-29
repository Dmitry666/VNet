#include "VirtualNetwork.h"
#include "VirtualNetworkPrivate.h"

#include "TcpClient.h"

#include <functional>

namespace vnet {

struct VirtualNetworkPool
{
	std::map<std::string, VirtualNetwork*> NameToNetwork;

	VirtualNetworkPool()
	{
	}

	~VirtualNetworkPool()
	{
		for (auto pair : NameToNetwork)
		{
			VirtualNetwork* virtualNetwork = pair.second;
			//virtualNetwork->Release();
			delete virtualNetwork;
		}
	}
};

VirtualNetworkPool networkPool;

VirtualNetwork::VirtualNetwork(const VirtualNetworkDesc& desc)
	: _virtualNetworkPrivate(std::make_shared<VirtualNetworkPrivate>(desc))
{
	_virtualNetworkPrivate->Authorizated += [this](VirtualNetworkPrivate* vnp, const VirtualNetworkInfoPrivate& infoPrivate){
		VirtualNetworkInfo info;
		info.VirtualAddress = infoPrivate.VirtualAddress;
		Authorizated.Invoke(this, info);
	};

	_virtualNetworkPrivate->ConnectError += [this](VirtualNetworkPrivate* vnp){
		ConnectError.Invoke(this);
	};

	_virtualNetworkPrivate->Logouted += [this](VirtualNetworkPrivate* vnp){
		Logouted.Invoke(this);
	};
}

VirtualNetwork::~VirtualNetwork()
{}

void VirtualNetwork::Initialize()
{
	if (!_virtualNetworkPrivate->IsInitialized())
	{
		_virtualNetworkPrivate->Initialize();
	}
}

void VirtualNetwork::Release()
{
	//if (_virtualNetworkPrivate->IsInitialized())
	{
		_virtualNetworkPrivate->Release();
	}
}

bool VirtualNetwork::IsInitialized() const
{
	return _virtualNetworkPrivate->IsInitialized();
}

VirtualNetwork& VirtualNetwork::Instance(const std::string& name)
{
	auto it = networkPool.NameToNetwork.find(name);
	if (it != networkPool.NameToNetwork.end())
	{
		VirtualNetwork* virtualNetwork = it->second;
		return *virtualNetwork;
	}

	VirtualNetworkDesc desc;
	VirtualNetwork* virtualNetwork = new VirtualNetwork(desc);
	networkPool.NameToNetwork.insert(std::make_pair(name, virtualNetwork));
	return *virtualNetwork;
}

VirtualNetwork& VirtualNetwork::Instance(const std::string& name, const VirtualNetworkDesc& desc)
{
	auto it = networkPool.NameToNetwork.find(name);
	if (it != networkPool.NameToNetwork.end())
	{
		VirtualNetwork* virtualNetwork = it->second;
		return *virtualNetwork;
	}

	VirtualNetwork* virtualNetwork = new VirtualNetwork(desc);
	networkPool.NameToNetwork.insert(std::make_pair(name, virtualNetwork));
	return *virtualNetwork;
}

} // End vnet.