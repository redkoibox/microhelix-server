#include "json_utils.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

bool isTableArray(lua_State *L, int index)
{
	bool isArray = false;
	lua_geti(L, index, 1);
	if (!lua_isnil(L, -1))
		isArray = true;
	lua_pop(L, 1);
	return isArray;
}

void serializeTable(rapidjson::Writer<rapidjson::StringBuffer>& writer, lua_State *L, int index)
{
	if (index < 0)
		index = lua_gettop(L) - (index + 1);
	bool serializingArray = isTableArray(L, index);
	if (serializingArray)
		writer.StartArray();
	else
		writer.StartObject();

	lua_pushnil(L);
	while (lua_next(L, index))
	{
		// Write key (but not for arrays).
		if (!serializingArray)
		{
			// Support only string key.
			// DONE add error in case of other type.
			writer.String(luaL_checkstring(L, -2));
		}

		// Write value.
		if (lua_isinteger(L, -1))
			writer.Int64(lua_tointeger(L, -1));
		else if (lua_isnumber(L, -1))
			writer.Double(lua_tonumber(L, -1));
		else if (lua_isstring(L, -1))
			writer.String(lua_tostring(L, -1));
		else if (lua_isboolean(L, -1))
			writer.Bool(lua_toboolean(L, -1) == 1);
		else if (lua_istable(L, -1))
			serializeTable(writer, L, -1);

		lua_pop(L, 1);
	}

	if (serializingArray)
		writer.EndArray();
	else
		writer.EndObject();
}

template<class ValueType>
void pushJsonValue(lua_State *L, ValueType& value)
{
	if (value.IsBool())
		lua_pushboolean(L, value.GetBool() ? 1 : 0);
	else if (value.IsDouble() || value.IsFloat())
		lua_pushnumber(L, value.IsFloat() ? value.GetFloat() : value.GetDouble());
	else if (value.IsInt() || value.IsInt64())
		lua_pushinteger(L, value.IsInt() ? value.GetInt() : value.GetInt64());
	else if (value.IsString())
		lua_pushstring(L, value.GetString());
	else if (value.IsObject())
		deserializeObject(L, value.MemberBegin(), value.MemberEnd());
	else if (value.IsArray())
		deserializeArray(L, value.Begin(), value.End());
}

template<class ItType>
void deserializeObject(lua_State* L, ItType begin, ItType end)
{
	lua_newtable(L);
	for (auto it = begin; it != end; ++it)
	{
		pushJsonValue(L, it->value);
		lua_setfield(L, -2, it->name.GetString());
	}
}

template<class ItType>
void deserializeArray(lua_State* L, ItType begin, ItType end)
{
	lua_newtable(L);
	lua_Integer arrayIndex = 1;
	for (auto it = begin; it != end; ++it)
	{
		pushJsonValue(L, *it);
		lua_seti(L, -2, arrayIndex++);
	}
}

bool json_utils::jsonToLuaTable(lua_State *L, const char* json)
{
	rapidjson::Document doc;
	doc.Parse(json);
	if (!doc.HasParseError())
	{
		deserializeObject(L, doc.MemberBegin(), doc.MemberEnd());
		return true;
	}
	return false;
}

std::string json_utils::luaTableToJson(lua_State* L, int index)
{
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	serializeTable(writer, L, index);
	return sb.GetString();
}
