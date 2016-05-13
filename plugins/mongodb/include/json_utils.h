#pragma once

#include <lua.hpp>
#include <string>

namespace json_utils
{
	bool jsonToLuaTable(lua_State *L, const char* json);
	std::string luaTableToJson(lua_State* L, int index);
}