/*
 * HTTPServer.cpp
 *
 * Created: 8/23/2021 12:44:59 AM
 *  Author: musialny
 */ 

#include "HTTPServer.h"
#include "Utilities.h"

constexpr int STATUS_PIN = 9;

HTTPRequest::HTTPRequest(String* url, String* headers, int headersCount, String* body, bool deleteBody) {
	this->url = url;
	this->headers = headers;
	this->headersCount = headersCount;
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

void HTTPServer::listen(void (*middleware)()) {
	EthernetClient client = server->available();
	if (client) {
		boolean currentLineIsBlank = true;
		auto rawRequest = new String("");
		while (client.connected()) {
			if (client.available()) {
				/*Serial.print(static_cast<char>(client.read()));
				char c = '';*/
				char c = client.read();
				if (c == '\n' && currentLineIsBlank) {
					String splitter('\n');
					auto parsedRequest = split(*rawRequest, splitter);
					delete rawRequest;
					auto url = new String("/");
					HTTPRequest request(url, parsedRequest->strings, parsedRequest->amount, url /*placeholder parameter*/, false);
					HTTPResponse* response = this->middlewares[0].middleware(request);
					delete parsedRequest;
					delete url;
					client.println("HTTP/1.1 " + String(response->statusCode) + " OK");
					for (int i = 0; i < response->headersCount; i++)
						client.println(response->headers[i].c_str());
					client.println("X-Powered-By: musialny.dev");
					client.println("Connection: close");
					client.println();
					client.println(response->body->c_str());
					delete response;
					break;
				}
				*rawRequest += c;
				if (c == '\n') currentLineIsBlank = true;
				else if (c != '\r') currentLineIsBlank = false;
			}
		}
		delay(1);
		client.stop();
		middleware();
	}
}
