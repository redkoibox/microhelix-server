--[[
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
--]]

local mongodb = require("mongodb");

_libmongo_mt = {};
_libmongo_mt.__index = _libmongo_mt;

function create()
	local wrapper = {};
	
	wrapper.new = function(uri)
		local pool = mongodb.newPool(uri);
		-- pool:setMaxIdleConnections(15);
		pool:setMaxCreableConnections(20);
		return setmetatable({ _pool = pool }, _libmongo_mt);
	end
	
	wrapper.randomUUID = function()
		return mongodb.randomUUID();
	end
	
	wrapper.withConnection = function(conn, func)
		local succes, results = pcall(
			function ()
				return { func(conn) };
			end
		);
		conn:disconnect();
		if(succes) then
			return table.unpack(results);
		else
			error(results);
		end
	end
	
	wrapper.withCollection = function(coll, func)
		local succes, results = pcall(
			function ()
				return { func(coll) };
			end
		);
		coll:close();
		if(succes) then
			return table.unpack(results);
		else
			error(results);
		end
	end

	wrapper.withCollections = function(coll, func)
		local succes, results = pcall(
			function ()
				return { func(coll) };
			end
		);
		for _,v in pairs(coll) do
			v:close();
		end
		if(succes) then
			return table.unpack(results);
		else
			error(results);
		end
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