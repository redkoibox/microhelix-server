#ifndef __HTTP_HELPERS_HEADER_INCLUDED__
#define __HTTP_HELPERS_HEADER_INCLUDED__

#include <lua.hpp>

#include "network.h"

namespace http_helpers
{
	enum class SupportedMediaTypes
	{
		TEXT_PLAIN,
		JSON,
		NONE
	};

	void sendGenericError(std::shared_ptr<NetworkManager::WebServer::Response> response);
	void sendResponse(std::shared_ptr<NetworkManager::WebServer::Response> response, lua_Integer errorCode, std::string const& content, SupportedMediaTypes mediaType);
	std::string luaTableToJson(lua_State* L, int index);
	bool jsonToLuaTable(lua_State *L, const char* json);
}

#endif
