#include "scripting.h"
#include "script.h"

std::unique_ptr<ScriptingManager> ScriptingManager::instance(new ScriptingManager());

ScriptingManager * ScriptingManager::getInstance()
{
	return instance.get();
}

void ScriptingManager::init(const std::string& scriptDirectory)
{
	// TODO load all scripts in file system.
	loadedScripts.emplace(std::make_pair("service.lua", Script::create(scriptDirectory + "/service.lua")));
	loadedScripts["service.lua"]->execute();
}
