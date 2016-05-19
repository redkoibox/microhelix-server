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
	void registerPath(HTTP_METHOD method, std::string const& path, std::function<void(WebServer::Response&, std::shared_ptr<WebServer::Request>)> op);
	void run(bool shouldRunInBackground = false);
private:
	WebServerPtr webServer;
};

#endif
