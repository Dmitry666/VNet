#pragma once

#ifdef WITH_NETWORKAPI
#include <controller.h>
#include <string>

namespace openrc {

class CommonController : public IController
{
	CONTROLLER_REGISTER(CommonController, "Common", "Common module.")

public:
	CONTROLLER_ACTION(CommonController, GetStatus)
	CONTROLLER_ACTION(CommonController, GetSessions)
};

} // End openrc.
#endif