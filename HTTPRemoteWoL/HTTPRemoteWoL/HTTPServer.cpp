/*
 * HTTPServer.cpp
 *
 * Created: 8/23/2021 12:44:59 AM
 *  Author: musialny
 */

#include "HTTPServer.h"
#include "Utilities.h"

HTTPRequest::HTTPRequest(String* url, String* urlParams, HTTPMethods method, HTTPHeaders* headers, String* body, void* data,  HTTPSendResponse* send, bool deleteBody) :
	url(url), urlParams(urlParams), method(method), headers(headers), body(body), data(data), send(send), deleteBody(deleteBody) {}

HTTPRequest::~HTTPRequest() {
	if (deleteBody)	{
		delete this->url;
		delete this->urlParams;
		delete[] this->headers;
		delete this->body;
		delete this->data;
		delete this->send;
	}
}

HTTPResponse::HTTPResponse(int statusCode, String* headers, int headersCount, String* body, bool deleteBody) : 
	statusCode(statusCode), headers(headers), headersCount(headersCount), body(body), deleteBody(deleteBody) {}

HTTPResponse::~HTTPResponse() {
	if (deleteBody)	{
		delete[] this->headers;
		delete this->body;
	}
}

HTTPSendResponse::HTTPSendResponse(EthernetClient& client) : client(client) {
	this->isBegin = false;
}

String* resolveStatusCode(int statusCode) {
	if (statusCode == 200)
		return new String(FlashStorage<char>(PSTR("200 OK"))());
	else if (statusCode == 401)
		return new String(FlashStorage<char>(PSTR("401 Unauthorized"))());
	else if (statusCode == 403)
		return new String(FlashStorage<char>(PSTR("403 Forbidden"))());
	else if (statusCode == 404)
		return new String(FlashStorage<char>(PSTR("404 Not Found"))());
	else if (statusCode == 406)
		return new String(FlashStorage<char>(PSTR("406 Not Acceptable"))());
	else if (statusCode == 405)
		return new String(FlashStorage<char>(PSTR("405 Method Not Allowed"))());
	else if (statusCode == 414)
		return new String(FlashStorage<char>(PSTR("414 URI Too Long"))());
	else if (statusCode == 431)
		return new String(FlashStorage<char>(PSTR("431 Request Header Fields Too Large"))());
	else if (statusCode == 413)
		return new String(FlashStorage<char>(PSTR("413 Payload Too Large"))());
	else return new String(FlashStorage<char>(PSTR("500 Internal Server Error"))());
}

void HTTPSendResponse::push(HTTPResponse* response, String* body) {
	if (response != nullptr) {
		if (!isBegin) {
			auto statusCode = resolveStatusCode(response->statusCode);
			client.println("HTTP/1.1 " + *statusCode);
			delete statusCode;
			for (int i = 0; i < response->headersCount; i++)
				client.println(response->headers[i]);
			client.println(FlashStorage<char>(PSTR("X-Powered-By: musialny.dev"))());
			client.println(FlashStorage<char>(PSTR("Connection: close"))());
			client.println();
			isBegin = true;
		}
		client.print(*response->body);
	} else if (body != nullptr) client.print(*body);
}

EthernetClient& HTTPSendResponse::getClient() {
	return this->client;
}

HTTPServer::HTTPServer(const byte deviceMacAddress[], const IPAddress& ip, bool useDHCP, int port, int STATUS_PIN) {
	this->middlewares = new ElasticArray<const HttpMiddleware*>;
	this->server = new EthernetServer(port);
	if (!useDHCP || !Ethernet.begin(const_cast<byte*>(deviceMacAddress)))
		Ethernet.begin(const_cast<byte*>(deviceMacAddress), ip);
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

Metadata* HTTPServer::parseMetadata(const String& requestLine) {
	auto split = Utilities::split(requestLine, " ");
	auto result = new Metadata;
	if (split->strings[0] == String("GET")) result->method = HTTPMethods::GET;
	else if (split->strings[0] == String("POST")) result->method = HTTPMethods::POST;
	else if (split->strings[0] == String("DELETE")) result->method = HTTPMethods::DELETE;
	else result->method = HTTPMethods::ALL;
	auto url = Utilities::split(split->strings[1], "?");
	if (url->amount) {
		result->url = url->strings[0];
		result->urlParams = url->strings[1];
	}
	else result->url = split->strings[1];
	delete split;
	delete url;
	return result;
}

void HTTPServer::parseHeaders(HTTPHeaders* headers, const String& requestLine) {
	constexpr int _headers = (sizeof(HTTPHeaders) - sizeof(IPAddress)) / sizeof(String);
	const String keys[_headers][2] = {{FlashStorage<char>(PSTR("Host: "))(), FlashStorage<char>(PSTR("host: "))()},
									  {FlashStorage<char>(PSTR("Content-Type: "))(), FlashStorage<char>(PSTR("content-type: "))()},
									  {FlashStorage<char>(PSTR("Authorization: "))(), FlashStorage<char>(PSTR("authorization: "))()}};
	for (int i = 0; i < _headers; i++) {
		for (int o = 0; o < 2; o++) {
			auto exists = Utilities::findAll(requestLine, keys[i][o]);
			if (exists->length()) {
				auto res = Utilities::split(requestLine, keys[i][o]);
				reinterpret_cast<String*>(headers)[i] = res->strings[1];
				delete exists;
				delete res;
				return;
			}
			delete exists;
		}
	}
}

void HTTPServer::processRequest(HTTPSendResponse& send, Metadata* metadata, HTTPHeaders* headers, String* body) {
	HTTPRequest request(&metadata->url, &metadata->urlParams, metadata->method, headers, body, nullptr, &send, false);
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
	if (response == nullptr) response = new HTTPResponse {500, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 500 }"))())};
	if (response->body == nullptr) send.push(nullptr, nullptr);
	else send.push(response);
	send.getClient().println();
	delete response;
}
