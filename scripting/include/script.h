#ifndef __SCRIPT_HEADER__INCLUDED__
#define __SCRIPT_HEADER__INCLUDED__

#include <string>
#include <memory>
#include <set>

#include "network.h"

struct lua_State;

int Helix_success(lua_State *L);
int Helix_error(lua_State *L);

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
private:
	void createDefaultScriptObjects();
	void registerNetworkFunctionsInService(const char* service);
	void registerMethod(const char* service, NetworkManager::HTTP_METHOD method);
private:
	std::set<NetworkManager::HTTP_METHOD> availableMethods;
	lua_State *L;
};

#endif
