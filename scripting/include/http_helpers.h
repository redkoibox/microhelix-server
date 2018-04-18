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
