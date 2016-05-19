#ifndef __CONFIG_MANAGER_INCLUDED__
#define __CONFIG_MANAGER_INCLUDED__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/serialization/singleton.hpp>

class ConfigManager
	: public boost::serialization::singleton<ConfigManager>
{
public:
	void loadConfigFile(std::string const& fileName);
	unsigned short listenPort() const;
	size_t threadPoolSize() const;
private:
	boost::property_tree::ptree properties;
};

#endif
