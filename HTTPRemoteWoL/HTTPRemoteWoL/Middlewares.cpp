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

// Flash Constants
const char ASTERIX[] PROGMEM = "*";
const char HTML_BEGIN[] PROGMEM = "<!DOCTYPE HTML><html><head><title>Control Panel</title></head><body>";
const char HTML_END[] PROGMEM = "</body></html>";
const char FORBIDDEN[] PROGMEM = "Forbidden";
const char INVALID_BODY[] PROGMEM = "Invalid request body";
namespace CONTENT_TYPE {
	const char TEXT_PLAIN[] PROGMEM = "Content-Type: text/plain";
	const char TEXT_HTML[] PROGMEM = "Content-Type: text/html";
	const char JSON[] PROGMEM = "Content-Type: application/json";
}

HttpMiddleware* Middlewares::auth() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>::getString(ASTERIX), [](HTTPRequest& request) -> HTTPResponse* {
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
						if (Utilities::compareFixedSizeArray(credentials->strings[0], user->username, sizeof(user->username)) &&
							Utilities::compareFixedSizeArray(credentials->strings[1], user->password, sizeof(user->password))) {
							delete credentials;
							request.data = new EEPROMStorage::UserMetadata(i, user->username, user->permissions);
							delete user;
							return nullptr;
						}
						delete user;
					}	
				}
				delete credentials;
				return new HTTPResponse({403, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(FORBIDDEN))});
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

HttpMiddleware* Middlewares::homePage() {
	return new HttpMiddleware {HTTPMethods::GET, FlashStorage<char>(PSTR("/"))(), [](HTTPRequest& request) -> HTTPResponse* {
		auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
		*resultBody += FlashStorage<char>(PSTR("<h1>Hello "))();
		*resultBody += String(reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->username);
		*resultBody += FlashStorage<char>(PSTR("</h1>"))();
		if (reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN)
			*resultBody += FlashStorage<char>(PSTR("<a href=\"/users\">Menage Users</a>"))();
		*resultBody += FlashStorage<char>::getString(HTML_END);
		return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
	}};
}

HttpMiddleware* Middlewares::users() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>(PSTR("/users"))(), [](HTTPRequest& request) -> HTTPResponse* {
		if (reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) {
			if (request.method == HTTPMethods::GET) {
				auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
				*resultBody += FlashStorage<char>(PSTR("<h3>User amount: "))();
				*resultBody += String(EEPROMStorage::getUsersAmount());
				*resultBody += FlashStorage<char>(PSTR("</h3>"))();
				*resultBody += FlashStorage<char>(PSTR("<form action=\"/users\" method=\"POST\">"))();
				*resultBody += FlashStorage<char>(PSTR("<label>Username</label>"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"text\" name=\"name\" required>"))();
				*resultBody += FlashStorage<char>(PSTR("<label>Password</label>"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"password\" name=\"password\" required>"))();
				*resultBody += FlashStorage<char>(PSTR("<label>Permission Level</label>"))();
				*resultBody += FlashStorage<char>(PSTR("<select name=\"permissions\" required>"))();
				*resultBody += FlashStorage<char>(PSTR("<option value=\"USER\">USER</option>"))();
				*resultBody += FlashStorage<char>(PSTR("<option value=\"ADMIN\">ADMIN</option>"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" value=\"Save\">"))();
				*resultBody += FlashStorage<char>(PSTR("</form><br>"))();
				auto chunk = new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
				request.send->push(chunk);
				delete chunk;
				resultBody = new String;
				byte registeredUsers = 0;
				for (byte i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
					auto user = EEPROMStorage::getUserCredentials(i);
					if (*user->username != '\0') {
						registeredUsers++;
						*resultBody += FlashStorage<char>(PSTR("<form action=\"/users\" method=\"DELETE\">"))();
						*resultBody += FlashStorage<char>(PSTR("<label> ID: "))();
						*resultBody += String(i);
						*resultBody += FlashStorage<char>(PSTR(" | Username: "))();
						byte o = 0;
						while (user->username[o] != '\0' && o < sizeof(user->username)) {
							*resultBody += user->username[o];
							o++;
						}
						*resultBody += FlashStorage<char>(PSTR(" | Permission Level: "))();
						*resultBody += String(static_cast<byte>(user->permissions));
						*resultBody += FlashStorage<char>(PSTR(" </label>"))();
						*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"ID\" value=\"Delete ID: "))();
						*resultBody += String(i);
						*resultBody += FlashStorage<char>(PSTR("\">"))();
						*resultBody += FlashStorage<char>(PSTR("</form>"))();
						request.send->push(nullptr, resultBody);
						*resultBody = "";
					}
					delete user;
				}
				*resultBody += FlashStorage<char>(PSTR("<h3>Registered users: "))();
				*resultBody += String(registeredUsers);
				*resultBody += FlashStorage<char>(PSTR("</h3>"))();
				*resultBody += FlashStorage<char>::getString(HTML_END);
				return new HTTPResponse {0, nullptr, 0, resultBody};
			} else if (request.method == HTTPMethods::POST) {
				auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
				*resultBody += FlashStorage<char>(PSTR("<h3>User added</h3><a href=\"/users\">Back</a>"))();
				*resultBody += FlashStorage<char>::getString(HTML_END);
				auto params = Utilities::split(*request.body, "&");
				if (params->amount == 3) {
					Utilities::SplittedString* parsedParams[] = {Utilities::split(params->strings[0], "="), Utilities::split(params->strings[1], "="), Utilities::split(params->strings[2], "=")};
					delete params;
					if (!(parsedParams[0]->strings[0] == FlashStorage<char>(PSTR("name"))() && 
						parsedParams[1]->strings[0] == FlashStorage<char>(PSTR("password"))() && 
						parsedParams[2]->strings[0] == FlashStorage<char>(PSTR("permissions"))() &&
						parsedParams[0]->strings[1].length() > 1 &&
						parsedParams[1]->strings[1].length() > 1 &&
						parsedParams[2]->strings[1].length() > 1)) {
							delete parsedParams[0];
							delete parsedParams[1];
							delete parsedParams[2];
							return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_BODY))};
						}
					bool result = EEPROMStorage::pushUser(EEPROMStorage::User(parsedParams[0]->strings[1], parsedParams[1]->strings[1], 
											parsedParams[2]->strings[1] == "ADMIN" ? EEPROMStorage::UserPermissions::ADMIN : EEPROMStorage::UserPermissions::USER));
					delete parsedParams[0];
					delete parsedParams[1];
					delete parsedParams[2];
					if (result) return new HTTPResponse {303, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
					else return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>(PSTR("Error while adding new user"))())};
				} else {
					delete params;
					return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_BODY))};
				}
			} else return nullptr;
		} else return new HTTPResponse {403, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(FORBIDDEN))};
	}};
}

HttpMiddleware* Middlewares::debugPage() {
	return new HttpMiddleware {HTTPMethods::GET, FlashStorage<char>(PSTR("/debugPage"))(), [](HTTPRequest& request) -> HTTPResponse* {
		auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
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
		*resultBody += FlashStorage<char>(PSTR("<h4>ID: "))();
		*resultBody += String(reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->id);
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>(PSTR("<h4>Username: "))();
		*resultBody += String(reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->username);
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>(PSTR("<h4>Permission Level: "))();
		*resultBody += String(static_cast<byte>(reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions));
		*resultBody += FlashStorage<char>(PSTR("</h4>"))();
		*resultBody += FlashStorage<char>::getString(HTML_END);
		return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
	}};
}

HttpMiddleware* Middlewares::wol() {
	return new HttpMiddleware {HTTPMethods::GET, FlashStorage<char>(PSTR("/wol"))(), [](HTTPRequest& request) -> HTTPResponse* {
		sendMagicPacket(*wolHandler, woLaddressesList[0]);
		return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::JSON)}, 1, new String(FlashStorage<char>(PSTR("{ \"Magic Packet Status\": \"Sended\" }"))())};
	}};
}

HttpMiddleware* Middlewares::notFound404() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>::getString(ASTERIX), [](HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {404, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::JSON)}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 404 }"))())};
	}};
}
