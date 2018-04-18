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

#ifndef __NETWORK_HEADER_INCLUDED__
#define __NETWORK_HEADER_INCLUDED__

#include "simpleweb/server_http.hpp"
#include <thread>
#include <memory>
#include <boost/serialization/singleton.hpp>

class NetworkManager
	: public boost::serialization::singleton<NetworkManager>
{
public:
	enum class HTTP_METHOD
	{
		kGET,
		kPOST,
		kPUT,
		kHEAD,
		kDELETE,
		kOPTIONS,
		kNONE
	};
public:
	using WebServer = SimpleWeb::Server<SimpleWeb::HTTP>;
	using WebServerPtr = std::shared_ptr<WebServer>;
public:
	void init(unsigned short port, size_t numThread);
	static std::string getMethodString(HTTP_METHOD method);
	static HTTP_METHOD getMethod(const char* mehodString);
	void registerPath(HTTP_METHOD method, std::string const& path, std::function<void(std::shared_ptr<WebServer::Response>, std::shared_ptr<WebServer::Request>)> op);
	void run(bool shouldRunInBackground = false);
private:
	WebServerPtr webServer;
};

#endif
