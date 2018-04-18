# **Micro~Helix - Micro services in Lua**

Micro~Helix is an open source (MIT license) multi platform (Windows, Linux) micro service server written in modern C++ (C++14) and scriptable with Lua.

## **Building and Installing**

Micro~Helix uses CMake as a build system.
In Windows use CMake-GUI to create build files and then compile the code with the appropriate method.  
For linux is provided a simple build script (build-linux.sh). When it is invoked it creates a build directory and generates all makefiles inside it.

```bash
./build-linux.sh
cd build
make
make install
```

The files are installed in the CMAKE_PREFIX_PATH directory. The Micro~Helix directory structure is as follows:

~~~
CMAKE_PREFIX_PATH
	|__microhelix
		|__bin
		|__|__microhelix
		|__config
		|__|__config.ini
		|__lua
		|  |__libs
		|__scripts
		|__|__service.lua
		|__|__string
		|__|__|__DELETE.lua
		|__|__|__GET.lua
		|__|__|__POST.lua
		|__|__|__PUT.lua
		|__plugins
		|__|__libmongodb.so (linux)
		|__|__mongodb.dll   (windows)
~~~

In the script directory is present a set of sample scripts that interacts with the bundled mongodb plugin
and shows how to implement a set of simple CRUD operations.

##  **Third-Party Libraries**

Micro~Helix includes the following header-only libraries (in the extra directory):

[Boost](https://www.boost.org/) - Boost C++ Libraries (Version 1.66.0 or higher).  
[Simple-Web-Server](https://github.com/eidheim/Simple-Web-Server) - A boost::asio backed implementation of a fast and asynchronous web server and client in C++.  
[RapidJSON](https://github.com/miloyip/rapidjson) - A simple and fast json library.  

## **Getting Started**

### **1. Service Mapping**

Before we can start to create our simple first service we need to focus on some core concepts on how Micro~Helix is designed.
The main entry point for a service is the <b>service.lua</b> script file. In this file you should list all the services you want to expose.  
Take a look at this sample file:

```lua
-- Setup CORS settings.
config = {}

config.backofficeService =
{
    ["https://test.example.com"] = 
    {
        AccessControlRequestMethod = "POST, GET, OPTIONS",
        AccessControlRequestHeaders = "authorization, content-type",
        AccessControlMaxAge = 86400
    },
    ["https://prod.example.com"] =
    {
        AccessControlRequestMethod = "POST, GET, OPTIONS",
        AccessControlRequestHeaders = "authorization, content-type",
        AccessControlMaxAge = 86400
    } 
}

-- Declare a service that serves on /backoffice/login endpoint 
-- and supports the POST method using specified CORS setup.
services["^\\/backoffice\\/login\\/?$"] = 
{
	Origins = CORSConfig.backofficeService,
	POST =
	{
		consumes = Helix.JSON, -- Consumes JSON
		produces = Helix.JSON, -- Produces JSON
		onRequest = Helix.script("bo-login") -- Loads the POST.lua script in the /scripts/bo-login directory (case sensitive on UNIX platforms).
	}
}

-- This service supports the POST and GET methods and uses a
-- regex to match parts of the endpoint URL (in this case a UUID).
services["^\\/mygame\\/boosters\\/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})?\\/?$"] = 
{
	POST =
	{
		consumes = Helix.JSON,
		produces = Helix.JSON,
		onRequest = Helix.script("mg-boosters")
	},
	GET =
	{
		consumes = Helix.TEXT_PLAIN,
		produces = Helix.JSON,
		onRequest = Helix.script("mg-boosters")
	}
}
```
The first thing done in this script is to setup the CORS settings for a backoffice login service (exposed via POST).
CORS is set up creating a table in which all strings entry should define a remote URL allowed to make request to the 
specified resources. For each URL entry you can specify methods, heades and access control max age.  
After CORS setup the scripts starts to define a list of services adding string entries in the C++ defined services table.
The string entry value should represent a regex (C++11 standard library regex format) and should represent the endpoint path 
relative to the server root. To define handled method you should add one entry for each supported method (GET, POST, PUT, DELETE, 
HEAD, OPTIONS) you want to implement.

Each entry must evaluate to a lua table containing three string entries:

+ consumes
+ produces
+ onRequest

The consumes entry denfines how should the C++ core interpret request body.  
Valid values are:  
1. **Helix.TEXT_PLAIN**  
  Pass body data as simple text string.
2. **Helix.JSON**  
  Automatically converts body from json (if valid) to a matching lua table.

The produces entry defines how should the C++ core convert the return value of the service.  
Valid values are:  
1. **Helix.TEXT_PLAIN**  
  The return value **must** be a string and is directly written to the body response.
2. **Helix.JSON**  
  Return value **must** be a lua table that is automatically converted to a json string written to the body response.

The onRequest entry **must** be assigned with the result of the *Helix.script* function. The Helix.script function expects one string parameter representing the sub-folder relative to the service.lua file where the scripts for implemented HTTP methods are located.  
The scripts in the folder should be named after the HTTP method they implement (e.g. GET => GET.lua, POST => POST.lua).

**WARNING**  
The HTTP method script file are case sensitive, so the method name should be uppercase and the file extension lowercase!


### **2. Service Implementation**

Now that we have defined the service endpoints and HTTP methods supported is time to implement some functionality.  
Let's see the implementation of a simple login service (for the enpoint defined in the above example */backoffice/login/*)

```lua
local mongodb = require("libs/mongodb"); -- Core mongodb library.
local config = require("configs/bo-config"); -- Misc configurations settings.
local sha1 = require("commons/sha1"); -- Support library for hashing (https://github.com/kikito/sha.lua)
local SessionManager = require("commons/sessionmanager"); -- Auth session manager.

function doLogin(loginData, colls)
     local passwordHash = sha1(loginData.password);
     local userData = colls.users:find({ username = loginData.username, password = passwordHash })[1];
     if(userData ~= nil) then
        local result, sessionId = SessionManager.createSession(colls.sessions);
        if(result) then
            return Helix.success({ sessionId = sessionId });
        end
        return Helix.error(500, { message = "Error creating session" });
     end
     return Helix.error(401, { message = "Unauthorized" });
end

-- Service entry point.
function onRequest(headers, matches, body)
	-- Check if the api key passed on the headers is valid.
    if(headers["Authorization"] ~= config.myapi.key) then
        return Helix.error(403, { message = "Unauthorized" });
    end

    return mongodb.withConnection(mongodb.new(config.mongo.connString):connect(), 
		function(mdbConn)
            return mongodb.withCollections({ users = mdbConn:getCollection("backoffice", "users"), sessions = mdbConn:getCollection("backoffice", "sessions") },
                function(colls)
                    return doLogin(body, colls);
                end
            );
		end
	);
end
```

The example above implements a simple login service that uses mongodb (throught the microhelix core plugin) to store login and session informations. 
The session manager is a simple object (implemented using lua metatable system) that create and validate sessions with simple expiration logic.  
As explained in the **Service Mapping** section the onRequest method is the service entry point called by the C++ core when a request on a matching 
endpoint and method is made.

The onRequest methods takes in input the following parameters:

1. *headers*  
  A lua table containing all the request headers (indexed with strings).
2. *matches*  
  The matches found in the endpoint regex. (matches starts at index 1, index 0 is the full endpoint string).
3. *body*  
  The request body. Can be a string or a lua table depending on the value of the consumes settings of the service endpoint.

The onRequest method can return a string or a lua table depending on what kind of content the services should produce 
(string for text/plain and lua table for application/json).

### **3. Utility Functions**

```lua
Helix.script(scriptDir);
```

This function is used in service definitions to define which subfolder should implement a specific HTTP method for the endpoint.
It's return value should be assigned to onRequest entry of a service endpoint method definition.

```lua
Helix.success(content);
```

This function is used to return a success value from a service endpoint onRequest function implementation.  
The content parameter can be a string in case a service produces text/plain content or a lua table for application/json.

```lua
Helix.errors(errorCode, content);
```

This function is used to return an error value from a service endpoint onRequest function implementation.  
The errorCode parameter is the HTTP error code to return.  
The content parameter can be a string in case a service produces text/plain content or a lua table for application/json.

This is a list of hard-coded errors:

+ (401) **Helix.errors.UNAUTHORIZED**
+ (403) **Helix.errors.FORBIDDEN**
+ (404) **Helix.errors.NOT_FOUND**
+ (500) **Helix.errors.SERVER_ERROR**
+ (503) **Helix.errors.SERVICE_NOT_AVAILABLE**

```lua
Helix.utils.json.parse(string);
```

This function converts a string to a lua table.  
The string parameter is the string to convert.  
Returns a lua table mapped from the json string.  

## **Micro~Helix Extensions**

Micro~Helix has a set of standard extensions implemented as a combination of native code and lua code. Actually there are three extension implemented.
A MongoDB library implemented both in C and Lua, an RSA Public Key signature verification library implemented in C (using OpenSSL) and a simple unit testing
library implemented in Lua (usefull for writing healt check endpoints).

### **1. The MongoDB Library**

The MongoDB library is a wrapper of the mongo native C driver with a Lua layer that adds automatic pool management.

The following functions are available for use on connections:

```lua
--[[
	Opens a connection using the connection string defined
	  in object contruction (mongodb.new) and returns self.
--]]
connection:connect()

--[[ 
	Returns the connection to the pool.
	Don't call it if you use the withConnection wrapper function.
--]]
connection:disconnect()

--[[
	Opens the requested collection on the specified database.
	dbName -> Name of the database in which the collection resides.
	collectionName -> Name of the collection  to open.
]]
connection:getCollection(dbName, collectionName)
```

The following functions are available for use on collections:

```lua
-- Closes the collection
collection:close()

--[[
	Inserts a new row in the collection.
	bsonDocument -> The new document to insert.
	Returns a boolean indicating success.
--]]
collection:insert(bsonDocument)

--[[
	Finds rows matching query.
	bsonQuery -> The query to issue.
	skip -> Rows to skip.
	limit -> Max rows to retrieve.
	skip and limit are used to implement pagination of results.
	Returns a lua table (indexed array starting from 1) with all
	  documents retrieved in case of success or nil and a string on
	  error.
]]
collection:find(bsonQuery, skip, limit)

--[[
	Count rows matching query.
	bsonQuery -> The query to issue.
	skip -> Rows to skip.
	limit -> Max rows to retrieve.
	skip and limit are used to implement pagination of results.
	Returns an integer representing the counted rows onsuccess
	  or nil and a string on error.
]]
collection:count(bsonQuery, skip, limit)

--[[
	Deletes rows matching query.
	bsonQuery -> The query to issue.
	Returns a boolean indicating success.
]]
collection:delete()

--[[
	Executes the update directive specified in the bsonUpdate table
	  on rows matching query.
	bsonQuery -> The query to issue.
	bsonUpdate -> The update to apply (supports special directives like $inc, etc...).
	Returns a boolean indicating success and a table with the WriteResults mongo data
	  in case of completion or a error string in case of error.
--]]
collection:update(bsonQuery, bsonUpdate)

--[[
	Runs the passed pipeline aggregation function on the collection.
	bsonPipeline -> A lua table representing the bson pipeline for aggregation.
	Returns a lua table (indexed array starting from 1) with all
	  documents retrieved in case of success or nil and a string on
	  error.
--]]
collection:aggregate(bsonPipeline)
```

The following utility functions are available for use from the mongodb library:

```lua
-- First thing to do is to import the mongodb library.
local mongodb = require("libs/mongodb");

--[[
	Creates a new connection associated to a pool for the specified connection string.
	connectionString -> The connection string to use for connection.
--]]
mongodb.new(connectionString)

--[[
	Generates a random UUID usable with mongodb uuid fields.
	Returns a string representig the UUID.
--]]
mongodb.randomUUID()

--[[
	mongodb.withConnection(connection, function(connection)) -> Wrapper function that takes a connection
	  as the first parameter and a function to invoke using the connection.
	The function is called in a protected environment which handles errors
	  and automatically closes the connection on function return.
--]]
mongodb.withConnection(connection, 
	function(conn) 
	end
)

--[[
	mongodb.withCollection(collection, function(collection)) -> Wrapper function that takes a collection
	  and pass it to a function called in a protected environment wich handles errors and closes the
	  collection on function return.
--]]
mongodb.withCollection(collection, 
	function(coll) 
	end
)

--[[
	mongodb.withCollection(collections, function(collections)) -> Wrapper function that takes a table containing
	  collection and pass it to a function called in a protected environment wich handles errors and closes the
	  collections on function return.
--]]
mongodb.withCollections(collections, 
	function(colls) 
	end
)
```

The connection pool system has full multi thread support and is thread safe.  
The following lua code shows how to use the library for mongodb interaction:

```lua
local mongodb = require("libs/mongodb"); -- Imports the wrapper lua library.

mongodb.withConnection(mongodb.new(config.mongo.connString):connect(), 
	function(conn)
		return mongodb.withCollection(conn:getCollection("taplection", "users"),
			function(coll)
				local users = coll:find({ _id = 1});
				if(users ~= nil) then
					local success, details = coll:update({ _id = 1}, {["$inc"] = { version = 1 }});
				end
			end
		);
	end
);
```

### **2. The OpenSSL Public Key Signature Verification Library**

The public key signature verification library is a C extension created to allow a service implemented in microhelix to
easily verify SHA1 signatures encrypted with a RSA private key.

The library exposes an object with the following methods.

```lua
-- Loads the ssl library
local ssl = require("libs/ssl");

-- Creates a new public key from a string containing a the public key in PEM format.
local publicKey = ssl.newPublicKey(keyString);

--[[
	Verifies that the encrypted signature corresponds with the SHA1 hash of the payload text.
	payload -> The string containing the data on which the SHA1 signature should be verified.
	signature -> The encrypted signature of the payload data in BASE64 format.
	Returns a boolean indicating the success of the operation.
--]]
local success = publicKey:verify(payload, signature)

--[[
	Closes the public key and frees all allocated resources.
	Calling this method is optional. If it is not called the
	resources allocated by the public key native obejct will
	be released when the lua object is garbage collected.
--]]
publicKey:close()
```
The following code shows an example usage of the library to verify an Android in-app purchase receipt.

```lua
-- Create from the Android app IAP public key.
local pubKey = ssl.newPublicKey(
	"-----BEGIN PUBLIC KEY-----\n" ..
	"...\n" ..
	"-----END PUBLIC KEY-----"
);

local receipt = "..."; -- Assign to this the json string of and IAP receipt.
local signature = "..."; -- Assign to this the BASE64 encrypted signature of the IAP receipt.

if(pubKey:verify(receipt, signature)) then
	print("Valid receipt!")
else
	print("Invalid receipt!")
end
```

### **3. Unit Test Library**

The last library shipped with micro~helix is the unit test library. This library allows to write simple tests
to check that the written code work as expected. It can be used also to write simple service health endpoints.

The library exposes an object with the following methods.

```lua
-- Loads the unit test library.
local unitTest = require("libs/test");

--[[
	Register a named test function.
	name -> The test name (used to invoke specific test).
	function -> The test function. Can have an optional parameter
	  that can be passed on test invoke and must return two values
	  a boolean indicating success and a detail result (can be nil
	  a string or a table).
--]]
unitTest:registerTest(name,
	function(data)
		return success, details;
	end
)

--[[
	Runs the requested test (calls all registered tests if name is nil or empty string).
	name -> The test to run (can be nil or empty string to call all registered tests).
	data -> The data to pass to the test function.
	Returns the result of Helix.success in case of success and Helix.error in case of failure.
	  The content passed to the Helix functions is a table containing one entry per test
	  (indexed by test name). Each entry contains two string indexed sub-entries (result and desc)
	  containing the values returned from the test function.
--]]
unitTest:runTest(name, data)
```

The following code is a sample health check script that uses the unit test system to verify that the mongo database is reachable.

```lua
local UnitTest = require("libs/test");
local mongodb = require("libs/mongodb");
local connectionString = "..."; -- MongoDB connection string.

function onRequest(headers, matches)
    return UnitTest:runTest("mongoDB");
end

UnitTest:registerTest("mongoDB",
    function()
        return pcall(function()
            local mdbConn = mongodb.new(connectionString):connect();
            local collection = mdbConn:getCollection("database", "collection");
            
            return mongodb.withConnection(mdbConn, 
                function()
                    return mongodb.withCollection(collection,
                        function()
                            return collection:find({ _id = "" });
                        end
                    );
                end
            );
        end);
    end
);
```

## **LICENSE**

This project is licensed under the permissive MIT license.

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
