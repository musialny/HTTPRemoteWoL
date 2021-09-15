/*
 * Middlewares.cpp
 *
 * Created: 8/23/2021 3:39:27 PM
 *  Author: musialny
 */ 

#include "Middlewares.h"
#include "WoL.h"
#include "EEPROMStorage.h"
#include "Utilities.h"
#include "FlashStorage.h"

const byte woLaddressesList[][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
extern WoLHandler* wolHandler;

HttpMiddleware* Middlewares::auth() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>(PSTR("*"))(), [](HTTPRequest& request) -> HTTPResponse* {
		auto autho = Utilities::split(request.headers->authorization, FlashStorage<char>(PSTR(" "))());
		if (autho->amount == 2) {
			if (autho->strings[0] == FlashStorage<char>(PSTR("Basic"))()) {
				auto authorization = Utilities::decodeBASE64(autho->strings[1]);
				delete autho;
				auto credentials = Utilities::split(*authorization, FlashStorage<char>(PSTR(":"))());
				delete authorization;
				if (credentials->amount == 2) {
					for (int i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
						auto user = EEPROMStorage::getUserCredentials(i);
						if (credentials->strings[0] == user->username && credentials->strings[1] == user->password) {
							delete credentials;
							delete user;
							request.data = new byte {static_cast<byte>(user->permissions)};
							return nullptr;
						}
						delete user;
					}
				}
				delete credentials;
				return new HTTPResponse({403, new String[1] {FlashStorage<char>(PSTR("Content-Type: text/plain"))()}, 1, new String(FlashStorage<char>(PSTR("Forbidden"))())});
			}
			delete autho;
			return new HTTPResponse({406, new String[2] {FlashStorage<char>(PSTR("WWW-Authenticate: Basic realm=\"Authorization needed\""))(),
														 FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 2,
									 new String(FlashStorage<char>(PSTR("{ \"Authenticate Method\": \"Basic\" }"))())});
		}
		delete autho;
		return new HTTPResponse({401, new String[1] {FlashStorage<char>(PSTR("WWW-Authenticate: Basic realm=\"Authorization needed\", encoding=\"ASCII\""))()}, 1, new String});
	}};
}

HttpMiddleware* Middlewares::homePage(HTTPMethods method) {
	return new HttpMiddleware {method, FlashStorage<char>(PSTR("/"))(), [](HTTPRequest& request) -> HTTPResponse* {
		auto resultBody = new String(FlashStorage<char>(PSTR("<!DOCTYPE HTML><html><head><title>OwO</title></head><body>"))());
		*resultBody += FlashStorage<char>(PSTR("<h1>Method: "))();
		if (request.method == HTTPMethods::GET)
			*resultBody += FlashStorage<char>(PSTR("GET"))();
		else if (request.method == HTTPMethods::POST)
			*resultBody += FlashStorage<char>(PSTR("POST"))();
		else if (request.method == HTTPMethods::DELETE)
			*resultBody += FlashStorage<char>(PSTR("DELETE"))();
		*resultBody += FlashStorage<char>(PSTR("</h1>"))();
		*resultBody += FlashStorage<char>(PSTR("<h2>URL: "))();
		*resultBody += *request.url;
		*resultBody += FlashStorage<char>(PSTR("</h2>"))();
		*resultBody += FlashStorage<char>(PSTR("<h3>Content-Type: "))();
		*resultBody += request.headers->contentType;
		*resultBody += FlashStorage<char>(PSTR("</h3>"))();
		*resultBody += FlashStorage<char>(PSTR("<h4>Host: "))();
		*resultBody += request.headers->host;
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>(PSTR("<h4>Client IP: "))();
		for (int i = 0; i < 4; i++) {
			*resultBody += String(request.headers->ip[i]);
			if (i < 3) *resultBody += ".";
		}
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>(PSTR("<h4>Authorization: "))();
		*resultBody += request.headers->authorization;
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>(PSTR("<h4>Authorization (Decoded): "))();
		auto split = Utilities::split(request.headers->authorization, " ");
		auto authorization = Utilities::decodeBASE64(split->strings[1]);
		*resultBody += *authorization;
		delete authorization;
		delete split;
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>(PSTR("<h4>Body: "))();
		*resultBody += *request.body;
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>(PSTR("</body></html>"))();
		return new HTTPResponse {200, new String[1] {FlashStorage<char>(PSTR("Content-Type: text/html"))()}, 1, resultBody};
	}};
}

HttpMiddleware* Middlewares::subPage() {
	return new HttpMiddleware {HTTPMethods::POST, FlashStorage<char>(PSTR("/post"))(), [](HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {200, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"isJSON\": true }"))())};
	}};
}

HttpMiddleware* Middlewares::wol() {
	return new HttpMiddleware {HTTPMethods::GET, FlashStorage<char>(PSTR("/wol"))(), [](HTTPRequest& request) -> HTTPResponse* {
		sendMagicPacket(*wolHandler, woLaddressesList[0]);
		return new HTTPResponse {200, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"Magic Packet Status\": \"Sended\" }"))())};
	}};
}

HttpMiddleware* Middlewares::notFound404() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>(PSTR("*"))(), [](HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {404, new String[1] {FlashStorage<char>(PSTR("Content-Type: application/json"))()}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 404 }"))())};
	}};
}
