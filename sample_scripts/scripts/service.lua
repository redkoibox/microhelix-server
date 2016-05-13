local mongodb = require("libs/mongodb");

services["^\\/string\\??(.*)$"] = 
{
	GET =
	{
		consumes = Helix.TEXT_PLAIN,
		produces = Helix.JSON,
		onRequest = function(headers, matches, requestText)
			return Helix.error(Helix.Errors.NOT_FOUND, { message = "Requested data not found." });
		end
	},
	
	POST =
	{
		consumes = Helix.JSON,
		produces = Helix.JSON,
		--[[ 
			Method called when service endpoint is invoked.
			First parameter contains request headers (key -> value table).
			Second parameter contains the matches of path regex (capture groups, lua array starting from 1).
			Third parameters can be a string (service consumes text/plain) or a table (service consumes json).
		--]]
		onRequest = function(headers, matches, json)
			return Helix.success(json);
		end
	}
}
