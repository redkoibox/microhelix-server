#include "config_manager.h"

void ConfigManager::loadConfigFile(std::string const& fileName)
{
	boost::property_tree::ini_parser::read_ini(fileName, properties);
}

unsigned short ConfigManager::listenPort() const
{
	return properties.get<int16_t>("network.listen_port");
}

size_t ConfigManager::threadPoolSize() const
{
	return properties.get<size_t>("network.threading_pool_size");
}
