#include <iostream>
#include <sstream>

#include "microhelix_commons.h"
#include "config_manager.h"
#include "network.h"
#include "scripting.h"

int main(int argc, char *argv[])
{
	ConfigManager::get_mutable_instance().loadConfigFile("config/config.ini");
	NetworkManager::get_mutable_instance().init(ConfigManager::get_const_instance().listenPort(), ConfigManager::get_const_instance().threadPoolSize());
	ScriptingManager::get_mutable_instance().init("scripts");
	NetworkManager::get_mutable_instance().run();

	return 0;
}
