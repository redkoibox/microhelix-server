#include "luamongo.h"

typedef struct mongo_c_userdata
{
	mongoc_client_t *client;
} mongo_c_userdata, *lp_mongo_c_userdata;

typedef struct mongo_c_pool_userdata
{
	mongoc_client_pool_t *pool;
	mongoc_uri_t *uri;
} mongo_c_pool_userdata, *lp_mongo_c_pool_userdata;

typedef struct mongo_c_collection_userdata
{
	mongoc_collection_t *coll;
} mongo_c_collection_userdata, *lp_mongo_c_collection_userdata;

#define checkmongopool(L) \
	(lp_mongo_c_pool_userdata) luaL_checkudata(L, 1, "microhelix.mongodb_pool")

#define checkmongoconn(L) \
	(lp_mongo_c_userdata) luaL_checkudata(L, 1, "microhelix.mongodb_conn")

#define checkmongoconn_idx(L, i) \
	(lp_mongo_c_userdata) luaL_checkudata(L, i, "microhelix.mongodb_conn")

#define checkmongocollection(L) \
	(lp_mongo_c_collection_userdata) luaL_checkudata(L, 1, "microhelix.mongodb_collection")

static const struct luaL_Reg mongodblib_conn_m[] =
{
	{ "getCollection", mongodb_client_get_collection },
	{ NULL, NULL }
};

static const struct luaL_Reg mongodblib_collection_m[] =
{
	{ "close", mongodb_client_close_collection },
	{ NULL, NULL }
};

static const struct luaL_Reg mongodblib_f[] =
{
	{ "newPool", mongodb_newpool },
	{ NULL, NULL }
};

static const struct luaL_Reg mongodblib_pool_m[] =
{
	{ "__gc", mongodb_destroypool },
	{ "getConnection", mongodb_pool_get_connection },
	{ "releaseConnection", mongodb_pool_release_connection },
	{ NULL, NULL }
};

extern "C"
{
	int DLL_PUBLIC luaopen_mongodb(lua_State *L)
	{
		luaL_newmetatable(L, "microhelix.mongodb_pool");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		luaL_setfuncs(L, mongodblib_pool_m, 0);

		luaL_newmetatable(L, "microhelix.mongodb_conn");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		luaL_setfuncs(L, mongodblib_conn_m, 0);

		luaL_newmetatable(L, "microhelix.mongodb_collection");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		luaL_setfuncs(L, mongodblib_collection_m, 0);

		luaL_newlib(L, mongodblib_f);

		mongoc_init();

		return 1;
	}
}

int mongodb_newpool(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		lp_mongo_c_pool_userdata ptr = (lp_mongo_c_pool_userdata)lua_newuserdata(L, sizeof(mongo_c_pool_userdata));
		luaL_setmetatable(L, "microhelix.mongodb_pool");
		const MongoPoolData& data = MongoPoolManager::getInstance()->getPoolForUri(luaL_checkstring(L, 1));
		ptr->uri = data.uri;
		ptr->pool = data.pool;
		return 1;
	}
	else
		luaL_error(L, "MongoDB newPool(string) expects a string parameter (the connection URI).");
	return 0;
}

int mongodb_destroypool(lua_State *L)
{
	lp_mongo_c_pool_userdata ptr = checkmongopool(L);
	if (ptr != NULL)
	{
		MongoPoolManager::getInstance()->releasePool(ptr->uri);
		ptr->pool = NULL;
		ptr->uri = NULL;
	}
	return 0;
}

int mongodb_pool_get_connection(lua_State *L)
{
	lp_mongo_c_pool_userdata ptr = checkmongopool(L);
	if (ptr != NULL)
	{
		lp_mongo_c_userdata mongo_c_ptr = (lp_mongo_c_userdata)lua_newuserdata(L, sizeof(mongo_c_userdata));
		luaL_setmetatable(L, "microhelix.mongodb_conn");
		mongo_c_ptr->client = mongoc_client_pool_pop(ptr->pool);
		return 1;
	}
	return 0;
}

int mongodb_pool_release_connection(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		lp_mongo_c_pool_userdata ptr = checkmongopool(L);
		if (ptr != NULL)
		{
			lp_mongo_c_userdata c_ptr = checkmongoconn_idx(L, 2);
			mongoc_client_pool_push(ptr->pool, c_ptr->client);
			c_ptr->client = NULL;
			return 0;
		}
	}
	else
		luaL_error(L, "MongoDB-pool releaseConnection(pool, connection) expects 2 parameters (pool and connection).");
	return 0;
}

int mongodb_client_get_collection(lua_State *L)
{
	if (lua_gettop(L) >= 3)
	{
		lp_mongo_c_userdata ptr = checkmongoconn(L);
		lp_mongo_c_collection_userdata coll = (lp_mongo_c_collection_userdata)lua_newuserdata(L, sizeof(mongo_c_collection_userdata));
		coll->coll = mongoc_client_get_collection(ptr->client, luaL_checkstring(L, 2), luaL_checkstring(L, 3));
		luaL_setmetatable(L, "microhelix.mongodb_collection");
		return 1;
	}
	else
		luaL_error(L, "MongoDB-connection getCollection(connection, dbname, collectionname) expects 3 parameters (connection, database name and collection name).");
	return 0;
}

int mongodb_client_close_collection(lua_State *L)
{
	if (lua_gettop(L) >= 1)
	{
		lp_mongo_c_collection_userdata ptr = checkmongocollection(L);
		mongoc_collection_destroy(ptr->coll);
		ptr->coll = NULL;
		return 0;
	}
	else
		luaL_error(L, "MongoDB-collection close(collection) expects 1 parameter (collection).");
	return 0;
}

std::unique_ptr<MongoPoolManager> MongoPoolManager::instance(new MongoPoolManager());

MongoPoolManager* MongoPoolManager::getInstance()
{
	return instance.get();
}

MongoPoolData const& MongoPoolManager::getPoolForUri(const char *uri)
{
	std::lock_guard<std::mutex> lock(mutex);
	auto it = pools.find(uri);
	if (it != pools.end())
	{
		it->second.numRef += 1;
		return it->second;
	}
	mongoc_uri_t *uri_t = mongoc_uri_new(uri);
	mongoc_client_pool_t *pool = mongoc_client_pool_new(uri_t);
	pools.emplace(std::make_pair(mongoc_uri_get_string(uri_t), MongoPoolData(pool, uri_t)));
	return pools.at(mongoc_uri_get_string(uri_t));
}

void MongoPoolManager::releasePool(mongoc_uri_t *uri)
{
	std::lock_guard<std::mutex> lock(mutex);
	auto it = pools.find(mongoc_uri_get_string(uri));
	if (it != pools.end())
	{
		it->second.numRef -= 1;
		if (it->second.numRef == 0)
		{
			mongoc_client_pool_destroy(it->second.pool);
			mongoc_uri_destroy(it->second.uri);
		}
	}
}
