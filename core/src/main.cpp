#include <iostream>
#include <sstream>

#include "microhelix_commons.h"
#include "network.h"

int main(int argc, char *argv[])
{
	NetworkManager::getInstance()->registerPath(NetworkManager::HTTP_METHOD::kGET, "^/info$",
		[](NetworkManager::WebServer::Response& response, std::shared_ptr<NetworkManager::WebServer::Request> request)
		{
			std::stringstream content_stream;
			content_stream << "<h1>Request from " << request->remote_endpoint_address << " (" << request->remote_endpoint_port << ")</h1>";
			content_stream << request->method << " " << request->path << " HTTP/" << request->http_version << "<br>";
			for (auto& header : request->header) {
				content_stream << header.first << ": " << header.second << "<br>";
			}

			//find length of content_stream (length received using content_stream.tellp())
			content_stream.seekp(0, std::ios::end);

			response << "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n" << content_stream.rdbuf();
			response.flush();
		}
	);
	NetworkManager::getInstance()->run();
	return 0;
}
