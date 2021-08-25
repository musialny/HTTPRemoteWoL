/*
 * Middlewares.cpp
 *
 * Created: 8/23/2021 3:39:27 PM
 *  Author: musialny
 */ 

#include "Middlewares.h"

HttpMiddleware* Middlewares::getMiddlewares() {
	return new (HttpMiddleware[2]){{HTTPMethods::GET, String("/"), [](const HTTPRequest& request) -> HTTPResponse* {
		for (int i = 0; i < request.headersCount; i++)
			Serial.println("Result[" + String(i) + "] {length: " + String(request.headers[i].length())+ "} " + request.headers[i]);
		Serial.println();
		return new HTTPResponse {200, new String[1] {"Content-Type: text/html"}, 1, new String(String("<!DOCTYPE HTML><html><head><title>OwO</title></head><body><h1>GET</h1><pre>")/* + result->strings[1]*/ + "</pre></body></html>")};
	}}, {HTTPMethods::POST, String("/POST"), [](const HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {200, new String[1] {"Content-Type: application/json"}, 1, new String("{ isJSON: true }")};
}}};
}
