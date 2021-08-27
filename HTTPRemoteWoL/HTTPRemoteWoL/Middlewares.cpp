/*
 * Middlewares.cpp
 *
 * Created: 8/23/2021 3:39:27 PM
 *  Author: musialny
 */ 

#include "Middlewares.h"
#include "WoL.h"

const byte woLaddressesList[][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
extern WoLHandler* wol;

HttpMiddleware* Middlewares::getMiddlewares() {
	return new (HttpMiddleware[2]){{HTTPMethods::GET, String("/"), [](const HTTPRequest& request) -> HTTPResponse* {
		auto resultBody = new String("<!DOCTYPE HTML><html><head><title>OwO</title></head><body>");
		*resultBody += "<h1>Method: ";
		if (request.method == HTTPMethods::GET)
			*resultBody += "GET";
		else if (request.method == HTTPMethods::POST)
			*resultBody += "POST";
		else if (request.method == HTTPMethods::DELETE)
			*resultBody += "DELETE";
		*resultBody += "</h1>";
		*resultBody += "<h2>URL: ";
		*resultBody += *request.url;
		*resultBody += "</h2>";
		*resultBody += "<h3>Content-Type: ";
		*resultBody += request.headers->contentType;
		*resultBody += "</h3>";
		*resultBody += "<h3>Host: ";
		*resultBody += request.headers->host;
		*resultBody += "</h4>";
		*resultBody += "<h3>Body: ";
		*resultBody += *request.body;
		*resultBody += "</h4>";
		*resultBody += "</body></html>";
		sendMagicPacket(*wol, woLaddressesList[0]);
		return new HTTPResponse {200, new String[1] {"Content-Type: text/html"}, 1, resultBody};
	}}, {HTTPMethods::POST, String("/POST"), [](const HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {200, new String[1] {"Content-Type: application/json"}, 1, new String("{ isJSON: true }")};
}}};
}
