#include <lua.hpp>
#include <sstream>

#include "http_helpers.h"
#include "script.h"

static const struct luaL_Reg helixlib_f[] =
{
	{ "success", Helix_success },
	{ "error", Helix_error },
	{ NULL, NULL }
};

int Helix_success(lua_State *L)
{
	if(lua_gettop(L) >= 1)
	{
		lua_newtable(L);
		lua_pushinteger(L, 200);
		lua_setfield(L, -2, "errorCode");

		if (lua_istable(L, 1))
		{
			lua_pushvalue(L, 1);
			lua_pushstring(L, "SUCCESS");
			lua_setfield(L, -2, "result");
			lua_setfield(L, -2, "content");
		}
		else if (lua_isstring(L, 1))
		{
			lua_pushvalue(L, 1);
			lua_setfield(L, -2, "content");
		}
		return 1;
	}
	else
	{
		lua_newtable(L);
		lua_pushinteger(L, 200);
		lua_setfield(L, -2, "errorCode");
		return 1;
	}
}

int Helix_error(lua_State *L)
{
	if (lua_gettop(L) >= 1)
	{
		if (lua_gettop(L) >= 2)
		{
			lua_newtable(L);
			lua_pushinteger(L, luaL_checkinteger(L, 1));
			lua_setfield(L, -2, "errorCode");

			if (lua_istable(L, 2))
			{
				lua_pushvalue(L, 2);
				lua_pushstring(L, "ERROR");
				lua_setfield(L, -2, "result");
				lua_setfield(L, -2, "content");
			}
			else if (lua_isstring(L, 2))
			{
				lua_pushvalue(L, 1);
				lua_setfield(L, -2, "content");
			}
			return 1;
		}
		else
		{
			lua_newtable(L);
			lua_pushinteger(L, lua_tointeger(L, 1));
			lua_setfield(L, -2, "errorCode");
			return 1;
		}
	}
	else
	{
		lua_newtable(L);
		lua_pushinteger(L, 500);
		lua_setfield(L, -2, "errorCode");

		lua_pushstring(L, "<html><head><title>Internal Server Error.</title></head><body><h1>Internal Server Error.</h1></body></html>");
		lua_setfield(L, -2, "content");
		return 1;
	}
}

Script::ptr Script::create(std::string const& scriptFile)
{
	return ptr(new Script(scriptFile));
}

Script::Script(const std::string& scriptFile)
{
	availableMethods = { 
		NetworkManager::HTTP_METHOD::kDELETE,
		NetworkManager::HTTP_METHOD::kGET,
		NetworkManager::HTTP_METHOD::kHEAD,
		NetworkManager::HTTP_METHOD::kOPTIONS,
		NetworkManager::HTTP_METHOD::kPOST,
		NetworkManager::HTTP_METHOD::kPUT
	};
	L = luaL_newstate();
	luaL_openlibs(L);
	if (luaL_loadfile(L, scriptFile.c_str()) != LUA_OK)
		printf("Error loading lua script: %s\n", luaL_checkstring(L, -1));
	else
		createDefaultScriptObjects();
}

void Script::execute()
{
	if (lua_pcall(L, 0, 0, 0) == LUA_OK)
	{
		lua_getglobal(L, "services");
		lua_pushnil(L);
		while (lua_next(L, -2))
		{
			if (lua_istable(L, -1) && lua_isstring(L, -2))
			{
				// TODO add automatic API versioning system.
				registerNetworkFunctionsInService(lua_tostring(L, -2));
			}
			lua_pop(L, 1);
		}
	}
	else
	{
		printf("Error executing script: %s\n", luaL_checkstring(L, -1));
		lua_pop(L, 1);
	}
}

void Script::createDefaultScriptObjects()
{
	lua_newtable(L);
	
	luaL_setfuncs(L, helixlib_f, 0);

	lua_pushinteger(L, static_cast<lua_Integer>(http_helpers::SupportedMediaTypes::TEXT_PLAIN));
	lua_setfield(L, -2, "TEXT_PLAIN");

	lua_pushinteger(L, static_cast<lua_Integer>(http_helpers::SupportedMediaTypes::JSON));
	lua_setfield(L, -2, "JSON");

	lua_newtable(L);
	
	lua_pushinteger(L, 401);
	lua_setfield(L, -2, "UNAUTHORIZED");

	lua_pushinteger(L, 403);
	lua_setfield(L, -2, "FORBIDDEN");

	lua_pushinteger(L, 404);
	lua_setfield(L, -2, "NOT_FOUND");

	lua_pushinteger(L, 503);
	lua_setfield(L, -2, "SERVICE_NOT_AVAILABLE");

	lua_setfield(L, -2, "Errors");

	lua_setglobal(L, "Helix");

	lua_newtable(L);
	lua_setglobal(L, "services");
}

void Script::registerNetworkFunctionsInService(const char* service)
{
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		if (lua_istable(L, -1) && lua_isstring(L, -2))
		{
			registerMethod(service, NetworkManager::getMethod(lua_tostring(L, -2)));
		}
		lua_pop(L, 1);
	}
	// TODO register remaining available method on service to send a Not Implemented error.
}

void Script::registerMethod(const char* service, NetworkManager::HTTP_METHOD method)
{
	http_helpers::SupportedMediaTypes consumes = http_helpers::SupportedMediaTypes::NONE, produces = http_helpers::SupportedMediaTypes::NONE;
	lua_getfield(L, -1, "consumes");
	if (!lua_isnil(L, -1))
		consumes = static_cast<http_helpers::SupportedMediaTypes>(luaL_checkinteger(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "produces");
	if (!lua_isnil(L, -1))
		produces = static_cast<http_helpers::SupportedMediaTypes>(luaL_checkinteger(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "onRequest");
	if (lua_isfunction(L, -1))
	{
		availableMethods.erase(method);
		int refId = luaL_ref(L, LUA_REGISTRYINDEX);
		std::string servicePath = service;
		NetworkManager::getInstance()->registerPath(method, service, 
			[L = L, refId, consumes, produces, servicePath](NetworkManager::WebServer::Response& response, std::shared_ptr<NetworkManager::WebServer::Request> request)
			{
				// DONE pass headers to function.
				// DONE pass query parameters to function (passed via regex matches, is up to lua script to match and parse).
				// DONE add support to json (consume).
				lua_rawgeti(L, LUA_REGISTRYINDEX, refId);

				// Push headers in a table.
				lua_newtable(L);
				for (auto& header : request->header)
				{
					lua_pushstring(L, header.second.c_str());
					lua_setfield(L, -2, header.first.c_str());
				}

				// Push regex matches.
				lua_newtable(L);
				for (int i = 1; i < request->path_match.size(); ++i)
				{
					lua_pushstring(L, request->path_match[i].str().c_str());
					lua_seti(L, -2, i);
				}

				if (consumes == http_helpers::SupportedMediaTypes::TEXT_PLAIN)
					lua_pushstring(L, request->content.string().c_str());
				else if (consumes == http_helpers::SupportedMediaTypes::JSON)
				{
					if (!http_helpers::jsonToLuaTable(L, request->content.string().c_str()))
					{
						http_helpers::sendResponse(response, 422, "Invalid json received.", http_helpers::SupportedMediaTypes::TEXT_PLAIN);
						lua_pop(L, 1);
						return;
					}
				}
				else
					lua_pushnil(L);

				if (lua_pcall(L, 3, 1, 0) == LUA_OK)
				{
					if (lua_istable(L, -1))
					{
						lua_getfield(L, -1, "errorCode");
						lua_Integer errorCode = lua_tointeger(L, -1);
						lua_pop(L, 1);

						lua_getfield(L, -1, "content");
						if (lua_isstring(L, -1) && produces == http_helpers::SupportedMediaTypes::TEXT_PLAIN)
						{
							std::string str = lua_tostring(L, -1);
							lua_pop(L, 1);
							http_helpers::sendResponse(response, errorCode, str, produces);
						}
						else if (lua_istable(L, -1) && produces == http_helpers::SupportedMediaTypes::JSON)
						{
							// DONE serialize table to json.
							std::string json = http_helpers::luaTableToJson(L, -1);
							http_helpers::sendResponse(response, errorCode, json, produces);
						}
						else if (lua_isnil(L, -1)) // In case of nil send empty response.
						{
							http_helpers::sendResponse(response, errorCode, "", produces);
						}
						else
							http_helpers::sendGenericError(response); // TODO Improve error reporting.
					}
					else
						http_helpers::sendGenericError(response);
				}
				else
				{
					http_helpers::sendGenericError(response);
					printf("Error executing onRequest for path (%s): %s\n", servicePath.c_str(), luaL_checkstring(L, -1));
				}
				lua_pop(L, 1);
			}
		);
	}
	else
		lua_pop(L, 1);
}
