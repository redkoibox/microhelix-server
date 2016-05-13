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

#include "microhelix_commons.h"

int mongodb_newpool(lua_State *L);
int mongodb_destroypool(lua_State *L);
int mongodb_pool_get_connection(lua_State *L);
int mongodb_pool_release_connection(lua_State *L);
int mongodb_client_get_collection(lua_State *L);
int mongodb_client_close_collection(lua_State *L);

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
{
public:
	static MongoPoolManager* getInstance();
	MongoPoolData const& getPoolForUri(const char* uri);
	void releasePool(mongoc_uri_t *uri);
private:
	static std::unique_ptr<MongoPoolManager> instance;
	std::mutex mutex;
	std::map<std::string, MongoPoolData> pools;
};

#endif
