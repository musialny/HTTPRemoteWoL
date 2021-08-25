/*
 * HTTPServer.h
 *
 * Created: 8/23/2021 12:45:38 AM
 *  Author: musialny
 */ 


#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <SPI.h>
#include <Ethernet.h>

enum class HTTPMethods {
	GET,
	POST,
	DELETE
};

struct HTTPRequest {
	String* url;
	String* headers;
	int headersCount;
	String* body;
	HTTPRequest(String* url = nullptr, String* headers = nullptr, int headersCount = 0, String* body = nullptr, bool deleteBody = true);
	~HTTPRequest();
private:
	bool deleteBody;
};

struct HTTPResponse {
	int statusCode;
	String* headers;
	int headersCount;
	String* body;
	HTTPResponse(int statusCode = 500, String* headers = nullptr, int headersCount = 0, String* body = nullptr, bool deleteBody = true);
	~HTTPResponse();
private:
	bool deleteBody;
};

struct HttpMiddleware {
	HTTPMethods method;
	String url;
	HTTPResponse* (*middleware)(const HTTPRequest& request);
};

class HTTPServer
{
private:
	HttpMiddleware* middlewares;
	EthernetServer* server;
public:
	HTTPServer(byte* deviceMacAddress, IPAddress* ip, int port = 80);
	~HTTPServer();
	
	void use(HttpMiddleware* middlewares);
	
	void listen(void (*middleware)());
};


#endif /* HTTPSERVER_H_ */
