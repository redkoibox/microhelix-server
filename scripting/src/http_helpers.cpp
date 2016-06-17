#include "http_helpers.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

static std::string httpProtocolAndVersion = "HTTP/1.1 ";
static std::map<lua_Integer, std::string> errorCodeToDescription{
	std::make_pair(100, "100 Continue\r\n"),
	std::make_pair(101, "101 Switching Protocols\r\n"),

	std::make_pair(200, "200 OK\r\n"),

	std::make_pair(201, "201 Created\r\n"),
	std::make_pair(202, "202 Accepted\r\n"),

	std::make_pair(203, "203 Non-Authoritative Information\r\n"),
	std::make_pair(204, "204 No Content\r\n"),
	std::make_pair(205, "205 Reset Content\r\n"),
	std::make_pair(206, "206 Partial Content\r\n"),
	std::make_pair(207, "207 Multi-Status\r\n"),

	std::make_pair(300, "300 Multiple Choices\r\n"),
	std::make_pair(301, "301 Moved Permanently\r\n"),

	std::make_pair(302, "302 Found\r\n"),

	std::make_pair(303, "303 See Other\r\n"),

	std::make_pair(304, "304 Not Modified\r\n"),
	std::make_pair(305, "305 Use Proxy\r\n"),

	std::make_pair(306, "306 Switch Proxy\r\n"),

	std::make_pair(307, "307 Temporary Redirect\r\n"),

	std::make_pair(308, "308 Permanent Redirect\r\n"),

	std::make_pair(400, "400 Bad Request\r\n"),

	std::make_pair(401, "401 Unauthorized\r\n"),

	std::make_pair(402, "402 Payment Required\r\n"),

	std::make_pair(403, "403 Forbidden\r\n"),

	std::make_pair(404, "404 Not Found\r\n"),

	std::make_pair(405, "405 Method Not Allowed\r\n"),

	std::make_pair(406, "406 Not Acceptable\r\n"),

	std::make_pair(407, "407 Proxy Authentication Required\r\n"),
	std::make_pair(408, "408 Request Timeout\r\n"),

	std::make_pair(409, "409 Conflict\r\n"),

	std::make_pair(410, "410 Gone\r\n"),

	std::make_pair(411, "411 Length Required\r\n"),

	std::make_pair(412, "412 Precondition Failed\r\n"),
	std::make_pair(413, "413 Request Entity Too Large\r\n"),

	std::make_pair(414, "414 Request-URI Too Long\r\n"),

	std::make_pair(415, "415 Unsupported Media Type\r\n"),

	std::make_pair(416, "416 Requested Range Not Satisfiable\r\n"),
	std::make_pair(417, "417 Expectation Failed\r\n"),
	std::make_pair(418, "418 I'm a teapot\r\n"),

	std::make_pair(422, "422 Unprocessable Entity\r\n"),

	std::make_pair(426, "426 Upgrade Required\r\n"),

	std::make_pair(449, "449 Retry With\r\n"),

	std::make_pair(451, "451 Unavailable For Legal Reasons\r\n"),

	std::make_pair(500, "500 Internal Server Error\r\n"),

	std::make_pair(501, "501 Not Implemented\r\n"),

	std::make_pair(502, "502 Bad Gateway\r\n"),
	std::make_pair(503, "503 Service Unavailable\r\n"),

	std::make_pair(504, "504 Gateway Timeout\r\n"),
	std::make_pair(505, "505 HTTP Version Not Supported\r\n"),

	std::make_pair(509, "509 Bandwidth Limit Exceeded\r\n")
};

void http_helpers::sendGenericError(NetworkManager::WebServer::Response& response)
{
	response << httpProtocolAndVersion << errorCodeToDescription[500];
	response << "X-Powered-By: micro-helix/0.1\r\n";
	response << "Content-Length: 0\r\n\r\n";
	response.flush();
}

void http_helpers::sendResponse(NetworkManager::WebServer::Response& response, lua_Integer errorCode, std::string const& content, SupportedMediaTypes mediaType)
{
	response << httpProtocolAndVersion << errorCodeToDescription[errorCode];
	response << "X-Powered-By: micro-helix/0.1\r\n";
	response << "Content-Type: ";
	switch (mediaType)
	{
	case http_helpers::SupportedMediaTypes::TEXT_PLAIN:
		response << " text/plain";
		break;
	case http_helpers::SupportedMediaTypes::JSON:
		response << " application/json";
		break;
	default:
		response << "text/plain";
		break;
	}
	response << "\r\n";
	response << "Content-Length: " << content.length() << "\r\n\r\n";
	response.flush();
	response << content;
	response.flush();
}

bool isTableArray(lua_State *L)
{
	if (luaL_len(L, -1) == 0)
		return true;
	
	bool isArray = false;
	lua_geti(L, -1, 1);
	if (!lua_isnil(L, -1))
		isArray = true;
	lua_pop(L, 1);
	return isArray;
}

void serializeTable(rapidjson::Writer<rapidjson::StringBuffer>& writer, lua_State *L)
{
	bool serializingArray = isTableArray(L);
	if (serializingArray)
		writer.StartArray();
	else
		writer.StartObject();

	lua_pushnil(L);
	while (lua_next(L, -2))
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
			serializeTable(writer, L);

		lua_pop(L, 1);
	}

	if (serializingArray)
		writer.EndArray();
	else
		writer.EndObject();
}

std::string http_helpers::luaTableToJson(lua_State* L, int index)
{
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	serializeTable(writer, L);
	return sb.GetString();
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

bool http_helpers::jsonToLuaTable(lua_State *L, const char* json)
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
