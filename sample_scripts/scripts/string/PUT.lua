local mongodb = require("libs/mongodb");

function onRequest(headers, matches, json)
	local mdbConn = mongodb.new("mongodb://localhost:27017/"):connect();
	local collection = mdbConn:getCollection("taplection", "users");
	local success, err = collection:update(json.query, json.data);
	collection:close();
	mdbConn:disconnect();
	if(success) then
		return Helix.success({});
	else
		return Helix.error(Helix.Errors.NOT_FOUND, { messsage = err });
	end
end
