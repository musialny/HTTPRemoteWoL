/*
 * Middlewares.cpp
 *
 * Created: 8/23/2021 3:39:27 PM
 *  Author: musialny
 */ 

#include "Middlewares.h"

HttpMiddleware* Middlewares::getMiddlewares() {
	return new (HttpMiddleware[2]){{HTTPMethods::GET, String("/"), [](const HTTPRequest& request) -> HTTPResponse* {
		auto resultBody = new String("<!DOCTYPE HTML><html><head><title>OwO</title></head><body><h1>GET</h1>");
		*resultBody += "<h2>";
		*resultBody += *request.url;
		*resultBody += "</h2>";
		*resultBody += "</body></html>";
		Serial.println(*resultBody);
		Serial.println();
		return new HTTPResponse {200, new String[1] {"Content-Type: text/html"}, 1, resultBody};
	}}, {HTTPMethods::POST, String("/POST"), [](const HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {200, new String[1] {"Content-Type: application/json"}, 1, new String("{ isJSON: true }")};
}}};
}
