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

#include "luamongo.h"

#include <boost/uuid/uuid_io.hpp>
#include "json_utils.h"

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
	{ "close", mongodb_collection_close },
	{ "insert", mongodb_collection_insert },
	{ "find", mongodb_collection_find },
	{ "count", mongodb_collection_count },
	{ "delete", mongodb_collection_delete },
	{ "update", mongodb_collection_update },
	{ "aggregate", mongodb_collection_aggregate },
	{ NULL, NULL }
};

static const struct luaL_Reg mongodblib_f[] =
{
	{ "newPool", mongodb_newpool },
	{ "randomUUID", mongodb_generate_uuid },
	{ NULL, NULL }
};

static const struct luaL_Reg mongodblib_pool_m[] =
{
	{ "__gc", mongodb_destroypool },
	{ "setMaxCreableConnections", mongodb_pool_set_max_connections },
	{ "setMaxIdleConnections", mongodb_pool_set_min_connections },
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
		const MongoPoolData& data = MongoPoolManager::get_mutable_instance().getPoolForUri(luaL_checkstring(L, 1));
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
		MongoPoolManager::get_mutable_instance().releasePool(ptr->uri);
		ptr->pool = NULL;
		ptr->uri = NULL;
	}
	return 0;
}

int mongodb_pool_set_min_connections(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		lp_mongo_c_pool_userdata ptr = checkmongopool(L);
		if (ptr != NULL)
		{
			mongoc_client_pool_min_size(ptr->pool, luaL_checkinteger(L, 2));
			return 0;
		}
	}
	else
		luaL_error(L, "MongoDB-pool setMaxConnections(pool, min) expects 2 parameters (pool and min number).");
	return 0;
}

int mongodb_pool_set_max_connections(lua_State *L)
{
	if (lua_gettop(L) == 2)
	{
		lp_mongo_c_pool_userdata ptr = checkmongopool(L);
		if (ptr != NULL)
		{
			mongoc_client_pool_max_size(ptr->pool, luaL_checkinteger(L, 2));
			return 0;
		}
	}
	else
		luaL_error(L, "MongoDB-pool setMaxConnections(pool, max) expects 2 parameters (pool and max number).");
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

int mongodb_collection_close(lua_State *L)
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

// Table is expected at stack top.
bson_t *createBSONFromLuaTable(lua_State *L, int index = -1)
{
	std::string json = json_utils::luaTableToJson(L, index);
	bson_error_t error;
	bson_t *document = bson_new_from_json((const uint8_t *)json.c_str(), -1, &error);
	return document;
}

int mongodb_collection_insert(lua_State *L)
{
	if (lua_gettop(L) >= 2)
	{
		lp_mongo_c_collection_userdata ptr = checkmongocollection(L);
		bson_t* document = createBSONFromLuaTable(L);
		if (document != NULL)
		{
			bson_error_t error;
			if (!mongoc_collection_insert(ptr->coll, MONGOC_INSERT_NONE, document, NULL, &error))
			{
				fprintf(stderr, "%s\n", error.message);
				lua_pushboolean(L, 0);
				lua_pushstring(L, error.message);
				return 2;
			}
			bson_destroy(document);
			lua_pushboolean(L, 1);
			char * str = bson_as_json(mongoc_collection_get_last_error(ptr->coll), NULL);
			json_utils::jsonToLuaTable(L, str);
			bson_free(str);
			return 2;
		}
		else
		{
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid BSON object (maybe oid is not in correct form?).");
			return 2;
		}
	}
	else
		luaL_error(L, "MongoDB-collection insert(collection, document) expects 2 parameter (collection, document).");
	return 0;
}

int mongodb_collection_find(lua_State *L)
{
	if (lua_gettop(L) >= 2)
	{
		uint32_t skip = 0;
		uint32_t limit = 0;
		lp_mongo_c_collection_userdata ptr = checkmongocollection(L);
		bson_t* query = createBSONFromLuaTable(L, 2);
		if (lua_gettop(L) >= 3)
			skip = luaL_checkinteger(L, 3);
		if (lua_gettop(L) >= 4)
			limit = luaL_checkinteger(L, 4);
		if (query != NULL)
		{
			mongoc_cursor_t *cursor;
			const bson_t *doc;
			char *str;
			lua_Integer idx = 1;
			cursor = mongoc_collection_find(ptr->coll, MONGOC_QUERY_NONE, skip, limit, 0, query, NULL, NULL);
			lua_newtable(L);
			while (mongoc_cursor_next(cursor, &doc))
			{
				str = bson_as_json(doc, NULL);
				json_utils::jsonToLuaTable(L, str);
				lua_seti(L, -2, idx++);
				bson_free(str);
			}
			bson_destroy(query);
			mongoc_cursor_destroy(cursor);
			return 1;
		}
		else
		{
			lua_pushnil(L);
			lua_pushstring(L, "Invalid BSON object (maybe oid is not in correct form?).");
			return 2;
		}
	}
	else
		luaL_error(L, "MongoDB-collection find(collection, document) expects 2 parameter (collection, document).");
	return 0;
}

int mongodb_collection_count(lua_State *L)
{
	if (lua_gettop(L) >= 2)
	{
		uint32_t skip = 0;
		uint32_t limit = 0;
		lp_mongo_c_collection_userdata ptr = checkmongocollection(L);
		bson_t* query = createBSONFromLuaTable(L, 2);
		if (lua_gettop(L) >= 3)
			skip = luaL_checkinteger(L, 3);
		if (lua_gettop(L) >= 4)
			limit = luaL_checkinteger(L, 4);
		if (query != NULL)
		{
			lua_pushinteger(L, mongoc_collection_count(ptr->coll, MONGOC_QUERY_NONE, query, skip, limit, NULL, NULL));
			bson_destroy(query);
			return 1;
		}
		else
		{
			lua_pushnil(L);
			lua_pushstring(L, "Invalid BSON object (maybe oid is not in correct form?).");
			return 2;
		}
	}
	else
		luaL_error(L, "MongoDB-collection count(collection, document) expects 2 parameter (collection, document).");
	return 0;
}

int mongodb_collection_aggregate(lua_State *L)
{
	if (lua_gettop(L) >= 2)
	{
		lp_mongo_c_collection_userdata ptr = checkmongocollection(L);
		bson_t* pipeline = createBSONFromLuaTable(L);
		if (pipeline != NULL)
		{
			mongoc_cursor_t *cursor;
			const bson_t *doc;
			char *str;
			lua_Integer idx = 1;
			cursor = mongoc_collection_aggregate(ptr->coll, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
			lua_newtable(L);
			while (mongoc_cursor_next(cursor, &doc))
			{
				str = bson_as_json(doc, NULL);
				json_utils::jsonToLuaTable(L, str);
				lua_seti(L, -2, idx++);
				bson_free(str);
			}
			bson_destroy(pipeline);
			mongoc_cursor_destroy(cursor);
			return 1;
		}
		else
		{
			lua_pushnil(L);
			lua_pushstring(L, "Invalid BSON object (maybe oid is not in correct form?).");
			return 2;
		}
	}
	else
		luaL_error(L, "MongoDB-collection aggregate(collection, pipeline) expects 2 parameter (collection, pipeline).");
	return 0;
}

int mongodb_collection_delete(lua_State *L)
{
	if (lua_gettop(L) >= 2)
	{
		lp_mongo_c_collection_userdata ptr = checkmongocollection(L);
		bson_error_t error;
		bson_t* query = createBSONFromLuaTable(L);
		if (query != NULL)
		{
			if (!mongoc_collection_remove(ptr->coll, MONGOC_REMOVE_NONE, query, NULL, &error))
			{
				fprintf(stderr, "Delete failed: %s\n", error.message);
				lua_pushboolean(L, 0);
				lua_pushstring(L, error.message);
				return 2;
			}
		}
		else
		{
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid BSON object (maybe oid is not in correct form?)."); 
			return 2;
		}
		bson_destroy(query);
		lua_pushboolean(L, 1);
		return 1;
	}
	else
		luaL_error(L, "MongoDB-collection delete(collection, document) expects 2 parameter (collection, document).");
	return 0;
}

int mongodb_collection_update(lua_State *L)
{
	if (lua_gettop(L) >= 2)
	{
		if (lua_gettop(L) >= 3)
		{

		}
		lp_mongo_c_collection_userdata ptr = checkmongocollection(L);
		bson_error_t error;
		bson_t* query = createBSONFromLuaTable(L, 2);
		bson_t* update = createBSONFromLuaTable(L, 3);
		if (query != NULL && update != NULL)
		{
			if (!mongoc_collection_update(ptr->coll, MONGOC_UPDATE_NONE, query, update, NULL, &error))
			{
				fprintf(stderr, "Update failed: %s\n", error.message);
				lua_pushboolean(L, 0);
				lua_pushstring(L, error.message);
				return 2;
			}
		}
		else
		{
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Invalid BSON object (maybe oid is not in correct form?).");
			return 2;
		}
		bson_destroy(query);
		bson_destroy(update);
		lua_pushboolean(L, 1);
		char * str = bson_as_json(mongoc_collection_get_last_error(ptr->coll), NULL);
		json_utils::jsonToLuaTable(L, str);
		bson_free(str);
		return 2;
	}
	else
		luaL_error(L, "MongoDB-collection update(collection, document, update) expects 3 parameter (collection, document, update).");
	return 0;
}

int mongodb_generate_uuid(lua_State *L)
{
	lua_pushstring(L, MongoPoolManager::get_mutable_instance().generateRandomUUID().c_str());
	return 1;
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

std::string MongoPoolManager::generateRandomUUID()
{
	return boost::uuids::to_string(randomUUIDGenerator());
}
