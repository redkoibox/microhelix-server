#include "network.h"

std::string NetworkManager::getMethodString(HTTP_METHOD method)
{
	switch (method)
	{
	case NetworkManager::HTTP_METHOD::kGET:
		return "GET";
	case NetworkManager::HTTP_METHOD::kPOST:
		return "POST";
	case NetworkManager::HTTP_METHOD::kPUT:
		return "PUT";
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

NetworkManager::HTTP_METHOD NetworkManager::getMethod(const char* mehodString)
{
	if(strcmp(mehodString, "GET") == 0)
		return NetworkManager::HTTP_METHOD::kGET;
	else if (strcmp(mehodString, "POST") == 0)
		return NetworkManager::HTTP_METHOD::kPOST;
	else if (strcmp(mehodString, "PUT") == 0)
		return NetworkManager::HTTP_METHOD::kPUT;
	else if (strcmp(mehodString, "HEAD") == 0)
		return NetworkManager::HTTP_METHOD::kHEAD;
	else if (strcmp(mehodString, "DELETE") == 0)
		return NetworkManager::HTTP_METHOD::kDELETE;
	else if (strcmp(mehodString, "OPTIONS") == 0)
		return NetworkManager::HTTP_METHOD::kOPTIONS;
	return NetworkManager::HTTP_METHOD::kNONE;
}

void NetworkManager::registerPath(HTTP_METHOD method, std::string const& path, std::function<void(std::shared_ptr<WebServer::Response>, std::shared_ptr<WebServer::Request>)> op)
{
	webServer->resource[path][getMethodString(method)] = op;
}

void NetworkManager::run(bool shouldRunInBackground)
{
	// TODO find a better system to run server in background.
	if (shouldRunInBackground)
		std::thread thread([this]() { webServer->start(); });
	else
		webServer->start();
}

void NetworkManager::init(unsigned short port, size_t numThread)
{
	webServer = std::make_shared<WebServer>(port, numThread);
	webServer->default_resource["GET"] = [](std::shared_ptr<WebServer::Response> response, std::shared_ptr<WebServer::Request>)
	{
		static std::string httpProtocolAndVersion = "HTTP/1.1 ";
		static std::string content = "<h1>404 - Not Found</h1><hr /><i>Powered by micro~helix.</i>";
		*response << httpProtocolAndVersion << "404 Not Found\r\n";
		*response << "X-Powered-By: micro-helix/0.1\r\n";
		*response << "Content-Length: " << content.size() << "\r\n\r\n" << content;
		response->flush();
	};
}
