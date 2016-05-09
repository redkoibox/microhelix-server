#include "network.h"

std::string getMethodString(NetworkManager::HTTP_METHOD method)
{
	switch (method)
	{
	case NetworkManager::HTTP_METHOD::kGET:
		return "GET";
	case NetworkManager::HTTP_METHOD::kPOST:
		return "POST";
	case NetworkManager::HTTP_METHOD::kPUT:
		return "PUT";
		break;
	case NetworkManager::HTTP_METHOD::kHEAD:
		return "HEAD";
	case NetworkManager::HTTP_METHOD::kDELETE:
		return "DELETE";
	case NetworkManager::HTTP_METHOD::kOPTIONS:
		return "OPTIONS";
	default:
		return "GET";
	}
}

std::unique_ptr<NetworkManager> NetworkManager::instance = std::unique_ptr<NetworkManager>(new NetworkManager());

NetworkManager* NetworkManager::getInstance()
{
	return instance.get();
}

void NetworkManager::registerPath(HTTP_METHOD method, std::string const& path, std::function<void(WebServer::Response&, std::shared_ptr<WebServer::Request>)> op)
{
	webServer.resource[path][getMethodString(method)] = op;
}

void NetworkManager::run(bool shouldRunInBackground)
{
	// TODO find a better system to run server in background.
	if (shouldRunInBackground)
		std::thread thread([this]() { webServer.start(); });
	else
		webServer.start();
}

// TODO load config from file.
NetworkManager::NetworkManager()
	: webServer(8080, 4)
{
}
