#ifndef __SCRIPT_HEADER__INCLUDED__
#define __SCRIPT_HEADER__INCLUDED__

#include <string>
#include <memory>
#include <set>
#include <thread>
#include <mutex>
#include <map>
#include <vector>

#include "network.h"

struct lua_State;

int Helix_success(lua_State *L);
int Helix_error(lua_State *L);
int Helix_script(lua_State *L);

class Script 
	: public std::enable_shared_from_this<Script>
{
public:
	using ptr = std::shared_ptr<Script>;

	static ptr create(std::string const& scriptFile);
private:
	Script(const std::string& scriptFile);
public:
	void execute();
	lua_State* getStateForThread(const char* service, std::string const& helixScriptFile, NetworkManager::HTTP_METHOD method);
private:
	void registerNetworkFunctionsInService(const char* service);
	void registerMethod(const char* service, NetworkManager::HTTP_METHOD method);
	void createDefaultScriptObjects(lua_State *L);
	void preloadScripts(int numThread, const char* service, std::string const& helixScriptFile, NetworkManager::HTTP_METHOD method);
private:
	std::set<NetworkManager::HTTP_METHOD> availableMethods;
	lua_State *L;
	std::mutex mutex;
	std::map<std::string, lua_State*> states;
	std::map<std::string, std::vector<lua_State*>> preloadedScripts;
};

#endif
