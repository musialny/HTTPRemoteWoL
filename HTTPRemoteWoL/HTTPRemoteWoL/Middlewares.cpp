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

extern WoLHandler* wolHandler;

// Flash Constants
const char ASTERIX[] PROGMEM = "*";
const char HTML_BEGIN[] PROGMEM = "<!DOCTYPE HTML><html><head><title>Control Panel</title></head><body>";
const char HTML_END[] PROGMEM = "</body></html>";
const char FORBIDDEN[] PROGMEM = "Forbidden";
const char INVALID_REQUEST[] PROGMEM = "Invalid request";
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
		auto chunk = new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
		request.send->push(chunk);
		delete chunk;
		resultBody = new String;
		if (reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) {
			*resultBody += FlashStorage<char>(PSTR("<a href=\"/users\">Menage Users</a>"))();
			*resultBody += FlashStorage<char>(PSTR("<br><br><form action=\"/wol\" method=\"POST\">"))();
			*resultBody += FlashStorage<char>(PSTR("<label>Mac Address </label>"))();
			*resultBody += FlashStorage<char>(PSTR("<input type=\"text\" name=\"address\" required>"))();
			*resultBody += FlashStorage<char>(PSTR("<label> Additional user </label>"))();
			*resultBody += FlashStorage<char>(PSTR("<select name=\"users\" required>"))();
			*resultBody += FlashStorage<char>(PSTR("<option value=\"null\">NULL</option>"))();
			request.send->push(nullptr, resultBody);
			*resultBody = "";
			for (byte i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
				String username;
				auto user = EEPROMStorage::getUserCredentials(i);
				if (user->username[0] == '\0') {
					delete user;
					break;
				}
				for (byte o = 0; o < sizeof(user->username); o++) {
					if (user->username[o] == '\0') break;
					username += user->username[o];
				}
				delete user;
				*resultBody += FlashStorage<char>(PSTR("<option value=\""))();
				*resultBody += username;
				*resultBody += FlashStorage<char>(PSTR("\">"))();
				*resultBody += username;
				*resultBody += FlashStorage<char>(PSTR("</option>"))();
				request.send->push(nullptr, resultBody);
				*resultBody = "";
			}
			*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"save\" value=\"Save\">"))();
			*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"invoke\" value=\"Invoke\">"))();
			*resultBody += FlashStorage<char>(PSTR("</form>"))();
			request.send->push(nullptr, resultBody);
			*resultBody = "";
		}
		for (int i = 0; i < EEPROMStorage::getMacAddressesAmount(); i++) {
			auto mac = EEPROMStorage::getNearestMacAddress(i);
			if (mac != nullptr) {
				*resultBody += FlashStorage<char>(PSTR("<p>ID: "))();
				*resultBody += String(i);
				*resultBody += FlashStorage<char>(PSTR(" | MAC Address: "))();
				for (byte o = 0; o < sizeof(mac->address); o++) {
					char hex[3];
					sprintf(hex, "%X", mac->address[o]);
					*resultBody += hex + (o < (sizeof(mac->address) - 1) ? ":" : String());
				}
				delete mac;
				*resultBody += FlashStorage<char>(PSTR("</p>"))();
				request.send->push(nullptr, resultBody);
				*resultBody = "";
			}
		}
		*resultBody += FlashStorage<char>::getString(HTML_END);
		return new HTTPResponse {0, nullptr, 0, resultBody};
	}};
}

HttpMiddleware* Middlewares::users() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>(PSTR("/users"))(), [](HTTPRequest& request) -> HTTPResponse* {
		if (reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) {
			if (request.method == HTTPMethods::GET) {
				auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
				*resultBody += FlashStorage<char>(PSTR("<a href=\"/\">Back</a><h3>User amount: "))();
				*resultBody += String(EEPROMStorage::getUsersAmount());
				*resultBody += FlashStorage<char>(PSTR("</h3>"))();
				*resultBody += FlashStorage<char>(PSTR("<form action=\"/users\" method=\"POST\">"))();
				*resultBody += FlashStorage<char>(PSTR("<label>Username </label>"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"text\" name=\"name\" required>"))();
				*resultBody += FlashStorage<char>(PSTR("<label> Password </label>"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"password\" name=\"password\" required>"))();
				*resultBody += FlashStorage<char>(PSTR("<label> Permission Level </label>"))();
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
						*resultBody += FlashStorage<char>(PSTR("<form action=\"/users\" method=\"POST\">"))();
						*resultBody += FlashStorage<char>(PSTR("<label> ID: "))();
						*resultBody += String(i);
						*resultBody += FlashStorage<char>(PSTR(" | Username: "))();
						for (byte o = 0; o < sizeof(user->username); o++) {
							if (user->username[o] == '\0') break;
							*resultBody += user->username[o];
						}
						*resultBody += FlashStorage<char>(PSTR(" | Permission Level: "))();
						*resultBody += String(static_cast<byte>(user->permissions));
						*resultBody += FlashStorage<char>(PSTR(" </label>"))();
						*resultBody += FlashStorage<char>(PSTR("<input type=\"hidden\" name=\"id\" value=\""))();
						*resultBody += String(i);
						*resultBody += FlashStorage<char>(PSTR("\">"))();
						*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" value=\"Delete\">"))();
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
				auto params = Utilities::split(*request.body, "&");
				if (params->amount == 3) {
					Utilities::SplittedString* parsedParams[] = {Utilities::split(params->strings[0], "="), Utilities::split(params->strings[1], "="), Utilities::split(params->strings[2], "=")};
					delete params;
					if (!(parsedParams[0]->amount == 2 && parsedParams[1]->amount == 2 && parsedParams[2]->amount == 2) &&
						!(parsedParams[0]->strings[0] == FlashStorage<char>(PSTR("name"))() &&
						parsedParams[1]->strings[0] == FlashStorage<char>(PSTR("password"))() &&
						parsedParams[2]->strings[0] == FlashStorage<char>(PSTR("permissions"))() &&
						parsedParams[0]->strings[1].length() > 1 &&
						parsedParams[1]->strings[1].length() > 1 &&
						parsedParams[2]->strings[1].length() > 1)) {
							delete parsedParams[0];
							delete parsedParams[1];
							delete parsedParams[2];
							return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
					}
					bool result = EEPROMStorage::pushUser(EEPROMStorage::User(parsedParams[0]->strings[1], parsedParams[1]->strings[1], 
											parsedParams[2]->strings[1] == FlashStorage<char>(PSTR("ADMIN"))() ? EEPROMStorage::UserPermissions::ADMIN : EEPROMStorage::UserPermissions::USER));
					delete parsedParams[0];
					delete parsedParams[1];
					delete parsedParams[2];
					auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
					*resultBody += FlashStorage<char>(PSTR("<h3>User added</h3><a href=\"/users\">Back</a>"))();
					*resultBody += FlashStorage<char>::getString(HTML_END);
					if (result) return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
					else {
						delete resultBody;
						return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>(PSTR("Error while adding new user"))())};
					}
				} else if (params->amount == 0) {
					delete params;
					auto parsedParam = Utilities::split(*request.body, "=");
					if (parsedParam->amount == 2 && parsedParam->strings[0] == FlashStorage<char>(PSTR("id"))() && parsedParam->strings[1].length() > 0) {
						byte id = atoi(parsedParam->strings[1].c_str());
						delete parsedParam;
						if (id < EEPROMStorage::getUsersAmount() && EEPROMStorage::removeUser(id)) {
							auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
							*resultBody += FlashStorage<char>(PSTR("<h3>User removed</h3><a href=\"/users\">Back</a>"))();
							*resultBody += FlashStorage<char>::getString(HTML_END);
							return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
						} else return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>(PSTR("Error while removing user"))())};
					} else {
						delete parsedParam;
						return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
					}
				} else {
					delete params;
					return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
				}
			} else return nullptr;
		} else return new HTTPResponse {403, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(FORBIDDEN))};
	}};
}

HttpMiddleware* Middlewares::wol() {
	return new HttpMiddleware {HTTPMethods::POST, FlashStorage<char>(PSTR("/wol"))(), [](HTTPRequest& request) -> HTTPResponse* {
		auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
		*resultBody += FlashStorage<char>(PSTR("<h1>"))();
		*resultBody += *request.body;
		*resultBody += FlashStorage<char>(PSTR("</h1>"))();
		*resultBody += FlashStorage<char>::getString(HTML_END);
		// sendMagicPacket(*wolHandler, woLaddressesList[0]);
		return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
	}};
}

HttpMiddleware* Middlewares::notFound404() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>::getString(ASTERIX), [](HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {404, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::JSON)}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 404 }"))())};
	}};
}
