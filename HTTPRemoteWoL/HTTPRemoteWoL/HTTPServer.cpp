/*
 * HTTPServer.cpp
 *
 * Created: 8/23/2021 12:44:59 AM
 *  Author: musialny
 */

#include "HTTPServer.h"
#include "Utilities.h"

constexpr int STATUS_PIN = 9;

HTTPRequest::HTTPRequest(String* url, HTTPMethods method, HTTPHeaders* headers, String* body, void* data, bool deleteBody) {
	this->url = url;
	this->method = method;
	this->headers = headers;
	this->body = body;
	this->data = data;
	this->deleteBody = deleteBody;
}

HTTPRequest::~HTTPRequest() {
	if (deleteBody)	{
		delete this->url;
		delete[] this->headers;
		delete this->body;
		delete this->data;
	}
}

HTTPResponse::HTTPResponse(int statusCode, String* headers, int headersCount, String* body, bool deleteBody) {
	this->statusCode = statusCode;
	this->headers = headers;
	this->headersCount = headersCount;
	this->body = body;
	this->deleteBody = deleteBody;
}

HTTPResponse::~HTTPResponse() {
	if (deleteBody)	{
		delete[] this->headers;
		delete this->body;
	}
}

HTTPServer::HTTPServer(byte* deviceMacAddress, IPAddress* ip, int port) {
	this->middlewares = new ElasticArray<const HttpMiddleware*>;
	this->server = new EthernetServer(port);
	Ethernet.begin(deviceMacAddress, *ip);
	pinMode(STATUS_PIN, OUTPUT);
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		while (true) {
			digitalWrite(STATUS_PIN, HIGH);
		}
	}
	boolean statusLedState = false;
	while(Ethernet.linkStatus() == LinkOFF) {
		if (!statusLedState) {
			digitalWrite(STATUS_PIN, HIGH);
			statusLedState = true;
			delay(500);
			} else {
			digitalWrite(STATUS_PIN, LOW);
			statusLedState = false;
			delay(500);
		}
	}
	digitalWrite(STATUS_PIN, LOW);
	
	this->server->begin();
}

HTTPServer::~HTTPServer() {
	delete this->server;
}

HTTPServer& HTTPServer::use(const HttpMiddleware* middleware) {
	this->middlewares->push(middleware);
	return *this;
}

Metadata* parseMetadata(const String& requestLine) {
	String splitter(' ');
	auto split = Utilities::split(requestLine, splitter);
	auto result = new Metadata;
	if (split->strings[0] == String("GET")) result->method = HTTPMethods::GET;
	else if (split->strings[0] == String("POST")) result->method = HTTPMethods::POST;
	else if (split->strings[0] == String("DELETE")) result->method = HTTPMethods::DELETE;
	result->url = split->strings[1];
	delete split;
	return result;
}

void parseHeaders(HTTPHeaders* headers, const String& requestLine) {
	constexpr int _headers = (sizeof(HTTPHeaders) - sizeof(IPAddress)) / sizeof(String);
	const String keys[_headers][2] = {{"Host: ", "host: "}, {"Content-Type: ", "content-type: "}};
	for (int i = 0; i < _headers; i++) {
		for (int o = 0; o < 2; o++) {
			auto exists = Utilities::findAll(requestLine, keys[i][o]);
			if (exists->length()) {
				auto res = Utilities::split(requestLine, " ");
				reinterpret_cast<String*>(headers)[i] = res->strings[1];
				delete exists;
				delete res;
				return;
			}
			delete exists;
		}
	}
}

void HTTPServer::listen() {
	auto client = server->available();
	if (client) {
		auto rawRequestLine = new String;
		int parsingStage = 0;
		Metadata* metadata = nullptr;
		HTTPHeaders* headers = new HTTPHeaders;
		headers->ip = client.remoteIP();
		String* body = nullptr;
		while (client.connected()) {
			if (client.available()) { 
				char c = client.read();
				if (c == '\n' && parsingStage < 2) {
					switch(parsingStage) {
						case 0:
							metadata = parseMetadata(*rawRequestLine);
							*rawRequestLine = "";
							parsingStage = 1;
						break;
						
						case 1:
							if (rawRequestLine->length() == 1) parsingStage = 2;
							else {
								parseHeaders(headers, *rawRequestLine);
								*rawRequestLine = "";
							}
						break;
					}
				} else if (c != '\r' && c != '\0') *rawRequestLine += c;
			} else {
				body = rawRequestLine;
				HTTPRequest request(&metadata->url, metadata->method, headers, body, nullptr, false);
				HTTPResponse* response = nullptr;
				for (int i = 0; i < this->middlewares->length(); i++) {
					if ((*this->middlewares)[i]->method == metadata->method || (*this->middlewares)[i]->method == HTTPMethods::ALL) {
						if ((*this->middlewares)[i]->url == metadata->url || (*this->middlewares)[i]->url == String("*")) {
							response = (*this->middlewares)[i]->middleware(request);
							if (response == nullptr) continue;
							else break;
						}
					}
				}
				delete request.data;
				delete metadata;
				delete headers;
				delete body;
				if (response == nullptr) response = new HTTPResponse {500, new String[1] {"Content-Type: application/json"}, 1, new String("{ Error: 500 }")};
				client.println("HTTP/1.1 " + String(response->statusCode) + " OK");
				for (int i = 0; i < response->headersCount; i++)
					client.println(response->headers[i]);
				client.println("X-Powered-By: musialny.dev");	
				client.println("Connection: close");
				client.println();
				client.println(*response->body);
				client.println();
				delete response;
				break;
			}
		}
		delay(1);
		client.stop();
	}
}
