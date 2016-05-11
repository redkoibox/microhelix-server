#ifndef __SCRIPTING_HEADER_INCLUDED__
#define __SCRIPTING_HEADER_INCLUDED__

#include <map>
#include <memory>
#include <string>

class Script;

class ScriptingManager
{
public:
	static ScriptingManager *getInstance();
	void init(const std::string& scriptDirectory);
private:
	ScriptingManager() = default;
private:
	static std::unique_ptr<ScriptingManager> instance;
	std::map<std::string, std::shared_ptr<Script>> loadedScripts;
};

#endif
