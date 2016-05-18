local mongodb = require("libs/mongodb");

function onRequest(headers, matches, requestText)
	local mdbConn = mongodb.new("mongodb://localhost:27017/"):connect();
	local collection = mdbConn:getCollection("taplection", "users");
	local result = collection:find({ _id = { ["$oid"] = "57359c4fad475040b80057d1" } });
	collection:close();
	mdbConn:disconnect();	
	return Helix.success({ data = result });
end
