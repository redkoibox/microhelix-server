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

#ifndef __LUA_MONGODB_HEADER_INCLUDED__
#define __LUA_MONGODB_HEADER_INCLUDED__

#include <lua.hpp>
#include <bson.h>
#include <mongoc.h>
#include <map>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/random.hpp>
#include <boost/serialization/singleton.hpp>

#include "microhelix_commons.h"

int mongodb_newpool(lua_State *L);
int mongodb_destroypool(lua_State *L);
int mongodb_pool_set_min_connections(lua_State *L);
int mongodb_pool_set_max_connections(lua_State *L);
int mongodb_pool_get_connection(lua_State *L);
int mongodb_pool_release_connection(lua_State *L);
int mongodb_client_get_collection(lua_State *L);
int mongodb_collection_close(lua_State *L);
int mongodb_collection_insert(lua_State *L);
int mongodb_collection_find(lua_State *L);
int mongodb_collection_count(lua_State *L);
int mongodb_collection_delete(lua_State *L);
int mongodb_collection_update(lua_State *L);
int mongodb_collection_aggregate(lua_State *L);
int mongodb_generate_uuid(lua_State *L);

typedef struct MongoPoolData
{
	MongoPoolData(mongoc_client_pool_t *_pool, mongoc_uri_t *_uri)
		: numRef(1), pool(_pool), uri(_uri)
	{
	}

	MongoPoolData(MongoPoolData&& rhs)
	{
		numRef = rhs.numRef;
		rhs.numRef = 0;
		pool = rhs.pool;
		rhs.pool = NULL;
		uri = rhs.uri;
		rhs.uri = NULL;
	}

	int numRef;
	mongoc_client_pool_t *pool;
	mongoc_uri_t *uri;
} MongoPoolData;

class MongoPoolManager
	: public boost::serialization::singleton<MongoPoolManager>
{
public:
	MongoPoolData const& getPoolForUri(const char* uri);
	void releasePool(mongoc_uri_t *uri);
	std::string generateRandomUUID();
private:
	std::mutex mutex;
	std::map<std::string, MongoPoolData> pools;
	boost::uuids::random_generator randomUUIDGenerator;
};

#endif
