#include "scripting.h"
#include "script.h"

void ScriptingManager::init(const std::string& scriptDirectory)
{
	// DONE load all scripts in file system.
	// Changed mind :-), there is only one entry point.
	loadedScripts.emplace(std::make_pair("service.lua", Script::create(scriptDirectory + "/service.lua")));
	loadedScripts["service.lua"]->execute();
}
 