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
#include "ElasticArray.h"

enum class HTTPMethods {
	ALL = -1,
	GET,
	POST,
	DELETE
};

struct Metadata {
	String url;
	HTTPMethods method;
};

struct HTTPHeaders {
	String host;
	String contentType;
	IPAddress ip;
};

struct HTTPRequest {
	String* url;
	HTTPMethods method;
	HTTPHeaders* headers;
	String* body;
	void* data;
	HTTPRequest(String* url = nullptr, HTTPMethods method = HTTPMethods::GET, HTTPHeaders* headers = nullptr, String* body = nullptr, void* data = nullptr, bool deleteBody = true);
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
	HTTPResponse* (*middleware)(HTTPRequest& request);
};

class HTTPServer
{
private:
	ElasticArray<const HttpMiddleware*>* middlewares;
	EthernetServer* server;
	
public:
	HTTPServer(byte* deviceMacAddress, IPAddress* ip, int port = 80);
	~HTTPServer();
	
	HTTPServer& use(const HttpMiddleware* middleware);
	void listen();
};

#endif /* HTTPSERVER_H_ */
