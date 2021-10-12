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
#include "FlashStorage.h"

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
	EthernetClient& getClient();
};

class HTTPServer {
private:
	ElasticArray<const HttpMiddleware*>* middlewares;
	EthernetServer* server;
	
	Metadata* parseMetadata(const String& requestLine);
	void parseHeaders(HTTPHeaders* headers, const String& requestLine);
	void processRequest(HTTPSendResponse& send, Metadata* metadata, HTTPHeaders* headers, String* body);
	
public:
	HTTPServer(const byte deviceMacAddress[6], const IPAddress& ip, bool useDHCP = true, int port = 80, int STATUS_PIN = 9);
	~HTTPServer();
	
	HTTPServer& use(const HttpMiddleware* middleware);
	template<int URI_MAX_SIZE, int HEADER_MAX_SIZE, int BODY_MAX_SIZE>
	void listen() {
		Ethernet.maintain();
		auto client = server->available();
		if (client) {
			HTTPSendResponse send(client);
			auto rawRequestLine = new String;
			int parsingStage = 0;
			Metadata* metadata = nullptr;
			HTTPHeaders* headers = new HTTPHeaders;
			headers->ip = client.remoteIP();
			while (client.connected()) {
				if (client.available()) {
					char c = client.read();
					if (c == '\n' && parsingStage < 2) {
						switch(parsingStage) {
							case 0:
								metadata = parseMetadata(*rawRequestLine);
								if (metadata->method == HTTPMethods::ALL) {
									delete metadata;
									delete headers;
									delete rawRequestLine;
									HTTPResponse* response = new HTTPResponse {405, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 405 }"))())};
									send.push(response);
									client.println();
									delete response;
									delay(1);
									client.stop();
									return;
								}
								*rawRequestLine = "";
								parsingStage = 1;
							break;
							
							case 1:
								if (rawRequestLine->length() == 1 && rawRequestLine->charAt(0) == '\r') {
									parsingStage = 2;
									*rawRequestLine = "";
								}
								else {
									parseHeaders(headers, *rawRequestLine);
									*rawRequestLine = "";
								}
							break;
						}
					} else if (c != '\0') {
						if (!rawRequestLine->length() && c == '\r')
							*rawRequestLine += c;
						if (c != '\r')
							*rawRequestLine += c;
						if (parsingStage == 0) {
							if (rawRequestLine->length() > URI_MAX_SIZE) {
								delete metadata;
								delete headers;
								delete rawRequestLine;
								HTTPResponse* response = new HTTPResponse {414, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 414 }"))())};
								send.push(response);
								client.println();
								delete response;
								delay(1);
								client.stop();
								return;
							}
						} else if (parsingStage == 1) {
							if (rawRequestLine->length() > HEADER_MAX_SIZE) {
								delete metadata;
								delete headers;
								delete rawRequestLine;
								HTTPResponse* response = new HTTPResponse {431, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 431 }"))())};
								send.push(response);
								client.println();
								delete response;
								delay(1);
								client.stop();
								return;
							}
						} else {
							if (rawRequestLine->length() > BODY_MAX_SIZE) {
								delete metadata;
								delete headers;
								delete rawRequestLine;
								HTTPResponse* response = new HTTPResponse {413, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 413 }"))())};
								send.push(response);
								client.println();
								delete response;
								delay(1);
								client.stop();
								return;
							}
						}
					}
				} else {
					processRequest(send, metadata, headers, rawRequestLine);
					break;
				}
			}
			delay(1);
			client.stop();
		}
	}
};

#endif /* HTTPSERVER_H_ */
