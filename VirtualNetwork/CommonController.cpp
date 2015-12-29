#include "CommonController.h"
#include "SessionManager.h"

#ifdef WITH_NETWORKAPI
#include <json/reader.h>

#include <sstream>
using namespace std;

namespace openrc {

CONTROLLER_REGISTERIMPL(CommonController)
bool CommonController::Construct()
{
    return true;
}

CONTROLLER_ACTIONIMPL(CommonController, GetStatus, "GetStatus", "GetStatus.")
void CommonController::GetStatus(SessionWeak sessionWeak, const ControllerArguments& arguments, ControllerOutput& outContents)
{
	auto session = sessionWeak.lock();

#if 0
	// Pack result.
	Json::Value meta;
	meta["code"] = 200;

	Json::Value body;

	// Finsih.
	Json::Value result;
	result["meta"] = meta;
	result["body"] = body;

	outContents.Append(result.toStyledString());
#endif

	outContents.Append("OK");
}

CONTROLLER_ACTIONIMPL(CommonController, GetSessions, "GetSessions", "GetSessions.")
void CommonController::GetSessions(SessionWeak sessionWeak, const ControllerArguments& arguments, ControllerOutput& outContents)
{
	auto session = sessionWeak.lock();

	outContents.Append("BEGIN\n");

	vnet::SessionManager& sessionManager = vnet::SessionManager::Instance();
	const std::map<vnet::SessionKey, vnet::SessionPtr>& sessions = sessionManager.GetSessions();
	for (auto& pair : sessions)
	{
		std::string text = pair.first.ToString() + ": " + pair.second->Address() + "\n";
		outContents.Append(text);
	}

	outContents.Append("END\n");
}

} // End openrc.
#endif