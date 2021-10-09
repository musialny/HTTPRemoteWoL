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
	String urlParams;
	HTTPMethods method;
};

struct HTTPHeaders {
	String host;
	String contentType;
	String authorization;
	IPAddress ip;
};

class HTTPSendResponse;
struct HTTPRequest {
	String* url;
	String* urlParams;
	HTTPMethods method;
	HTTPHeaders* headers;
	String* body;
	void* data;
	HTTPSendResponse* send;
	HTTPRequest(String* url = nullptr, String* urlParams = nullptr, HTTPMethods method = HTTPMethods::GET, HTTPHeaders* headers = nullptr, String* body = nullptr, void* data = nullptr, HTTPSendResponse* send = nullptr, bool deleteBody = true);
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

class HTTPSendResponse {
private:
	bool isBegin;
	EthernetClient& client;
	
public:
	HTTPSendResponse(EthernetClient& client);
	~HTTPSendResponse() = default;
	
	void push(HTTPResponse* response, String* body = nullptr);
};

class HTTPServer {
private:
	ElasticArray<const HttpMiddleware*>* middlewares;
	EthernetServer* server;
	
public:
	HTTPServer(const byte deviceMacAddress[6], const IPAddress& ip, int port = 80);
	~HTTPServer();
	
	HTTPServer& use(const HttpMiddleware* middleware);
	void listen();
};

#endif /* HTTPSERVER_H_ */
