#ifndef __NETWORK_HEADER_INCLUDED__
#define __NETWORK_HEADER_INCLUDED__

#include "simpleweb/server_http.hpp"
#include <thread>
#include <memory>

class NetworkManager
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
public:
	static NetworkManager* getInstance();
	static std::string getMethodString(HTTP_METHOD method);
	static HTTP_METHOD getMethod(const char* mehodString);
	void registerPath(HTTP_METHOD method, std::string const& path, std::function<void(WebServer::Response&, std::shared_ptr<WebServer::Request>)> op);
	void run(bool shouldRunInBackground = false);
private:
	NetworkManager();
	NetworkManager(const NetworkManager& rhs) = default;
	NetworkManager(NetworkManager&& rhs) = default;
	NetworkManager& operator=(const NetworkManager& rhs) = default;
	NetworkManager& operator=(NetworkManager&& rhs) = default;
private:
	static std::unique_ptr<NetworkManager> instance;
	WebServer webServer;
};

#endif
