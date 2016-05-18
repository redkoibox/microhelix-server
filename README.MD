# Micro~Helix - Micro services in Lua

Micro~Helix is a multi platform (Windows, Linux) micro service server written in modern C++ (C++14) and scriptable with Lua.

## Building and Installing

Micro~Helix uses CMake as a build system.
In Windows use CMake-GUI to create build files and then compile the code with the appropriate method.
For linux is provided a simple build script (build-linux.sh). When is invoked it creates a build directory
and creates all makefiles in it.

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

##  Third-Party Libraries

Micro~Helix includes the following header-only libraries (in the extra directory):

[Simple-Web-Server](https://github.com/eidheim/Simple-Web-Server) - A boost::asio backed implementation of a fast and asynchronous web server and client in C++.
[RapidJSON](https://github.com/miloyip/rapidjson) - A simple and fast json library.

## LICENSE

	TODO