local mongodb = require("libs/mongodb");

function onRequest(headers, matches, json)
	local mdbConn = mongodb.new("mongodb://localhost:27017/"):connect();
	local collection = mdbConn:getCollection("taplection", "users");
	local done, err = collection:insert(json);
	collection:close();
	mdbConn:disconnect();
	
	if(done) then
		return Helix.success({});
	else
		return Helix.error(Helix.Errors.SERVER_ERROR, { message = err });
	end
end
