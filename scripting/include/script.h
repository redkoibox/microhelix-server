/**
Copyright (c) 2018 Red Koi Box

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Author: Fabio Greotti (f.greotti@redkoibox.it)
**/

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
int Helix_utils_json_parse(lua_State *L);

class Script 
	: public std::enable_shared_from_this<Script>
{
public:
	using ptr = std::shared_ptr<Script>;

	static ptr create(size_t numThread, std::string const& scriptFile);
private:
	Script(size_t numThread, const std::string& scriptFile);
public:
	void execute();
	lua_State* getStateForThread(const char* service, std::string const& helixScriptFile, NetworkManager::HTTP_METHOD method);
private:
	void registerNetworkFunctionsInService(const char* service);
	void registerMethod(const char* service, NetworkManager::HTTP_METHOD method);
	void createDefaultScriptObjects(lua_State *L);
	void preloadScripts(size_t numThread, const char* service, std::string const& helixScriptFile, NetworkManager::HTTP_METHOD method);
private:
	size_t numThreads;
	std::set<NetworkManager::HTTP_METHOD> availableMethods;
	lua_State *L;
	std::mutex mutex;
	std::map<std::string, lua_State*> states;
	std::map<std::string, std::vector<lua_State*>> preloadedScripts;
};

#endif
