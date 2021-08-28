/*
 * HTTPServer.cpp
 *
 * Created: 8/23/2021 12:44:59 AM
 *  Author: musialny
 */ 

#include "HTTPServer.h"
#include "Utilities.h"

constexpr int STATUS_PIN = 9;

HTTPRequest::HTTPRequest(String* url, HTTPMethods method, HTTPHeaders* headers, String* body, bool deleteBody) {
	this->url = url;
	this->method = method;
	this->headers = headers;
	this->body = body;
	this->deleteBody = deleteBody;
}

HTTPRequest::~HTTPRequest() {
	if (deleteBody)	{
		delete this->url;
		delete[] this->headers;
		delete this->body;
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

void HTTPServer::use(HttpMiddleware* middlewares) {
	this->middlewares = middlewares;
}

struct Metadata {
	String url;
	HTTPMethods method;
};
Metadata* parseMetadata(const Utilities::SplittedString& splittedString) {
	auto splitter = new String(" ");
	auto split = Utilities::split(splittedString.strings[0], *splitter);
	delete splitter;
	auto result = new Metadata;
	if (split->strings[0] == String("GET")) result->method = HTTPMethods::GET;
	else if (split->strings[0] == String("POST")) result->method = HTTPMethods::POST;
	else if (split->strings[0] == String("DELETE")) result->method = HTTPMethods::DELETE;
	result->url = split->strings[1];
	delete split;
	return result;
}

HTTPHeaders* parseHeaders(const EthernetClient& client, const Utilities::SplittedString& splittedString) {
	auto result = new HTTPHeaders;
	String splitter(" ");
	constexpr int _headers = (sizeof(HTTPHeaders) - sizeof(IPAddress)) / sizeof(String);
	String headers[2][2] = {{"Host", "host"}, {"Content-Type", "content-type"}};
	for (int i = 0; i < splittedString.amount; i++) {
		if (splittedString.strings[i].length() == 1 && splittedString.strings[i] == String("\n")) break;
		for (int o = 0; o < _headers; o++) {
			int isBreaked = false;
			for (int p = 0; p < 2; p++) {
				auto findResult = Utilities::findAll(splittedString.strings[i], headers[o][p]);
				if (findResult->length()) {
					auto res = Utilities::split(splittedString.strings[i], splitter);
					reinterpret_cast<String*>(result)[o] = res->strings[1];
					delete res;
					delete findResult;
					isBreaked = true;
					break;
				}
				delete findResult;
			}
			if (isBreaked) break;
		}
	}
	result->ip = const_cast<EthernetClient*>(&client)->remoteIP();
	return result;
}

String* parseBody(const Utilities::SplittedString& splittedString) {
	String* result = new String(splittedString.strings[splittedString.amount - 1]);
	return result;
}

void HTTPServer::listen() {
	auto client = server->available();
	if (client) {
		auto rawRequest = new String("");
		while (client.connected()) {
			if (client.available()) *rawRequest += static_cast<char>(client.read());
			else {
				auto splitter = new String('\n');
				auto parsedRequest = Utilities::split(*rawRequest, *splitter);
				delete rawRequest;
				delete splitter;
				for (int i = 0; i < parsedRequest->amount; i++) {
					auto buffer = parsedRequest->strings[i].charAt(parsedRequest->strings[i].length() - 1);	
					if (buffer == '\r' || buffer == '\0') parsedRequest->strings[i].remove(parsedRequest->strings[i].length() - 1, 1);
				}
				auto metadata = parseMetadata(*parsedRequest);
				auto headers = parseHeaders(client, *parsedRequest);
				auto body = parseBody(*parsedRequest);
				delete parsedRequest;
				HTTPRequest request(&metadata->url, metadata->method, headers, body, false);
				HTTPResponse* response = this->middlewares[0].middleware(request);
				delete metadata;
				delete headers;
				delete body;
				client.println("HTTP/1.0 " + String(response->statusCode) + " OK");
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
