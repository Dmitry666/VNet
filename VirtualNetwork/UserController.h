#pragma once

#ifdef WITH_NETWORKAPI
#include <controller.h>
#include <string>

namespace openrc {

class UserController : public IController
{
	CONTROLLER_REGISTER(UserController, "User", "User module.")

public:
	CONTROLLER_ACTION(UserController, AddUser)
	CONTROLLER_ACTION(UserController, RemoveUser)
	CONTROLLER_ACTION(UserController, GetUsers)
};

} // End openrc.
#endif