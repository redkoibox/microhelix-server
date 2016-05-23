local mongodb = require("mongodb");

_libmongo_mt = {};
_libmongo_mt.__index = _libmongo_mt;

function create()
	local wrapper = {};
	
	wrapper.new = function(uri)
		return setmetatable({ _pool = mongodb.newPool(uri) }, _libmongo_mt);
	end
	
	wrapper.randomUUID = function()
		return mongodb.randomUUID();
	end
	
	wrapper.cleanup = function()
		-- TODO add manual clean-up (in c module).
		--[[
		for _,value : pairs(_libmongo_mt.pools) do
		end
		--]]
		_libmongo_mt.pools = nil;
	end
	
	return wrapper;
end

function _libmongo_mt:connect()
	if(self._conn == nil) then
		self._conn = self._pool:getConnection();
	else
		error("[MongoConnectionWrapper] Already connected!");
	end
	return self;
end

function _libmongo_mt:getCollection(dbName, collectionName)
	if(self._conn ~= nil) then
		return self._conn:getCollection(dbName, collectionName);
	else
		error("[MongoConnectionWrapper] Not connected!");
	end
	return nil;
end

--[[
	Closes connection an release it to the pool.
	The wrapper is still valid and can be connected again.
--]]
function _libmongo_mt:disconnect()
	if(self._conn ~= nil) then
		self._pool:releaseConnection(self._conn);
		self._conn = nil;
	else
		error("[MongoConnectionWrapper] Not connected!");
	end
	return self;
end

function _libmongo_mt:isConnected()
	return self._conn == nil;
end

return create();