#ifndef __SCRIPTING_HEADER_INCLUDED__
#define __SCRIPTING_HEADER_INCLUDED__

#include <map>
#include <memory>
#include <string>
#include <boost/serialization/singleton.hpp>

class Script;

class ScriptingManager
	: public boost::serialization::singleton<ScriptingManager>
{
public:
	void init(const std::string& scriptDirectory);
private:
	std::map<std::string, std::shared_ptr<Script>> loadedScripts;
};

#endif
