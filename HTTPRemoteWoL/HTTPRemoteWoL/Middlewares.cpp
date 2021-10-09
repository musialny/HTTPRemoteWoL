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

extern WoL::WoLHandler* wolHandler;

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
						if (user != nullptr && Utilities::compareFixedSizeArray(credentials->strings[0], user->username, sizeof(user->username)) &&
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
			*resultBody += FlashStorage<char>(PSTR("<label>Name </label>"))();
			*resultBody += FlashStorage<char>(PSTR("<input type=\"text\" name=\"name\" minlength=\"1\" maxlength=\"12\" required>"))();
			*resultBody += FlashStorage<char>(PSTR("<label> Mac Address </label>"))();
			*resultBody += FlashStorage<char>(PSTR("<input type=\"text\" name=\"address\" minlength=\"17\" maxlength=\"17\" required>"))();
			*resultBody += FlashStorage<char>(PSTR("<label> Additional user </label>"))();
			*resultBody += FlashStorage<char>(PSTR("<select name=\"users\" required>"))();
			*resultBody += FlashStorage<char>(PSTR("<option value=\"null\">NULL</option>"))();
			request.send->push(nullptr, resultBody);
			*resultBody = "";
			for (byte i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
				String username;
				auto user = EEPROMStorage::getUserCredentials(i);
				if (user == nullptr) continue;
				for (byte o = 0; o < sizeof(user->username); o++) {
					if (user->username[o] == '\0') break;
					username += user->username[o];
				}
				delete user;
				*resultBody += FlashStorage<char>(PSTR("<option value=\""))();
				*resultBody += String(i);
				*resultBody += FlashStorage<char>(PSTR("\">"))();
				*resultBody += username;
				*resultBody += FlashStorage<char>(PSTR("</option>"))();
				request.send->push(nullptr, resultBody);
				*resultBody = "";
			}
			*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"save\" value=\"Save\">"))();
			*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"invoke\" value=\"Invoke\">"))();
			*resultBody += FlashStorage<char>(PSTR("</form>"))();
			if (*request.urlParams == FlashStorage<char>(PSTR("showAll=true"))())
				*resultBody += FlashStorage<char>(PSTR("<a href=\"/\">Hide not assigned Mac Adresses</a>"))();
			else *resultBody += FlashStorage<char>(PSTR("<a href=\"/?showAll=true\">Show all Mac Adresses</a>"))();
			request.send->push(nullptr, resultBody);
			*resultBody = "";
		}
		*resultBody += FlashStorage<char>(PSTR("<br>"))();
		for (int i = 0; i < EEPROMStorage::getMacAddressesAmount(); i++) {
			auto mac = EEPROMStorage::getNearestMacAddress(i);
			if (mac != nullptr && ((*request.urlParams == FlashStorage<char>(PSTR("showAll=true"))() &&
									reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) ||
									Utilities::checkUserPerms(mac->permissions, reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->id))) {
				*resultBody += FlashStorage<char>(PSTR("<form action=\"/wol\" method=\"POST\"><label>ID: "))();
				*resultBody += String(i);
				*resultBody += FlashStorage<char>(PSTR(" | Name: "))();
				for (byte o = 0; o < sizeof(mac->name); o++) {
					if (mac->name[o] == '\0') break;
					*resultBody += mac->name[o];
				}
				*resultBody += FlashStorage<char>(PSTR(" | MAC Address: "))();
				for (byte o = 0; o < sizeof(mac->address); o++) {
					char hex[3];
					sprintf(hex, "%X", mac->address[o]);
					*resultBody += hex + (o < (sizeof(mac->address) - 1) ? ":" : String());
				}
				*resultBody += FlashStorage<char>(PSTR(" </label>"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"hidden\" name=\"id\" value=\""))();
				*resultBody += String(i);
				*resultBody += FlashStorage<char>(PSTR("\">"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"invoke\" value=\"Invoke\">"))();
				if (reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) {
					*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"delete\" value=\"Delete\">"))();
					*resultBody += FlashStorage<char>(PSTR("<input type=\"submit\" name=\"updatePerms\" value=\"Update Perms\">"))();
					*resultBody += FlashStorage<char>(PSTR("<input type=\"text\" name=\"perms\" value=\""))();
					for (int i = 0; i < mac->permissionsSize; i++)
						*resultBody += String(mac->permissions[i]) + (i < (mac->permissionsSize - 1) ? "|" : String());
					*resultBody += FlashStorage<char>(PSTR("\" minlength=\""))();
					*resultBody += String(mac->permissionsSize + (mac->permissionsSize - 1));
					*resultBody += FlashStorage<char>(PSTR("\" required>"))();
				}
				delete mac;
				*resultBody += FlashStorage<char>(PSTR("</form>"))();
				request.send->push(nullptr, resultBody);
				*resultBody = "";
			} else delete mac;
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
				*resultBody += FlashStorage<char>(PSTR("<input type=\"text\" name=\"name\" minlength=\"1\" maxlength=\"8\" required>"))();
				*resultBody += FlashStorage<char>(PSTR("<label> Password </label>"))();
				*resultBody += FlashStorage<char>(PSTR("<input type=\"password\" name=\"password\" minlength=\"1\" maxlength=\"12\" required>"))();
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
					if (user == nullptr) continue;
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
		auto params = Utilities::split(*request.body, "&");
		if (params->amount == 4 && reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) {
			Utilities::SplittedString* parsedParams[] = {Utilities::split(params->strings[0], "="), Utilities::split(params->strings[1], "="),
														 Utilities::split(params->strings[2], "="), Utilities::split(params->strings[3], "=")};
			delete params;
			if (parsedParams[0]->amount == 2 && parsedParams[1]->amount == 2 && parsedParams[2]->amount == 2 && parsedParams[3]->amount == 2 &&
				parsedParams[0]->strings[0] == FlashStorage<char>(PSTR("name"))() &&
				parsedParams[1]->strings[0] == FlashStorage<char>(PSTR("address"))() &&
				parsedParams[2]->strings[0] == FlashStorage<char>(PSTR("users"))() &&
				(parsedParams[3]->strings[0] == FlashStorage<char>(PSTR("invoke"))() || parsedParams[3]->strings[0] == FlashStorage<char>(PSTR("save"))()) &&
				parsedParams[0]->strings[1].length() > 0 &&
				parsedParams[1]->strings[1].length() > 0 &&
				parsedParams[2]->strings[1].length() > 0 &&
				parsedParams[3]->strings[1].length() > 0) {
					if (parsedParams[3]->strings[0] == FlashStorage<char>(PSTR("save"))()) {
						byte* mac = Utilities::parseUrlMacAddress(parsedParams[1]->strings[1]);
						if (mac == nullptr) {
							delete parsedParams[0];
							delete parsedParams[1];
							delete parsedParams[2];
							delete parsedParams[3];
							return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
						}
						char name[12];
						for (byte o = 0; o < sizeof(name); o++)
							name[o] = o >= parsedParams[0]->strings[1].length() ? '\0' : parsedParams[0]->strings[1].charAt(o);
						auto perms = new byte[Utilities::calculateBitFieldsAllocation(EEPROMStorage::getUsersAmount())] {};
						Utilities::setUserPerms(perms, reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->id);
						if (parsedParams[2]->strings[1] != FlashStorage<char>(PSTR("null"))())
							Utilities::setUserPerms(perms, atoi(parsedParams[2]->strings[1].c_str()));
						auto macId = EEPROMStorage::Mac(mac, name, perms).saveToEEPROM();
						delete parsedParams[0];
						delete parsedParams[1];
						delete parsedParams[2];
						delete parsedParams[3];
						delete[] perms;
						delete[] mac;
						if (macId == -1) {
							auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
							*resultBody += FlashStorage<char>(PSTR("<h3>Mac Addresses limit hitted</h3><a href=\"/\">Back</a>"))();
							*resultBody += FlashStorage<char>::getString(HTML_END);
							return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
						} else {
							auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
							*resultBody += FlashStorage<char>(PSTR("<h3>Mac added</h3><h4>ID: "))();
							*resultBody += String(macId);
							*resultBody += FlashStorage<char>(PSTR("</h4><a href=\"/\">Back</a>"))();
							*resultBody += FlashStorage<char>::getString(HTML_END);
							return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
						}
					} else {
						delete parsedParams[0];
						delete parsedParams[2];
						delete parsedParams[3];
						byte* mac = Utilities::parseUrlMacAddress(parsedParams[1]->strings[1]);
						delete parsedParams[1];
						if (mac == nullptr)
							return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
						WoL::sendMagicPacket(*wolHandler, mac);
						delete[] mac;
						auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
						*resultBody += FlashStorage<char>(PSTR("<h3>Magic Packet sent</h3><a href=\"/\">Back</a>"))();
						*resultBody += FlashStorage<char>::getString(HTML_END);
						return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
					}
			} else return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
		} else if (params->amount == 3) {
			Utilities::SplittedString* parsedParam = Utilities::split(params->strings[0], "=");
			String param2 = params->strings[1];
			auto splitBuffer = Utilities::split(params->strings[2], "=");
			if (splitBuffer->amount != 2) {
				delete splitBuffer;
				delete params;
				return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
			}
			String perms = splitBuffer->strings[1];
			delete splitBuffer;
			delete params;
			if (parsedParam->amount == 2 && parsedParam->strings[0] == FlashStorage<char>(PSTR("id"))() && parsedParam->strings[1].length() > 0) {
				byte id = atoi(parsedParam->strings[1].c_str());
				delete parsedParam;
				if (param2 == FlashStorage<char>(PSTR("delete=Delete"))() && reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) {
					EEPROMStorage::removeNearestMacAddress(id);
					auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
					*resultBody += FlashStorage<char>(PSTR("<h3>Magic Packet Removed</h3><a href=\"/\">Back</a>"))();
					*resultBody += FlashStorage<char>::getString(HTML_END);
					return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
				} else if (param2 == FlashStorage<char>(PSTR("invoke=Invoke"))()) {
					auto mac = EEPROMStorage::getNearestMacAddress(id);
					if (mac != nullptr && Utilities::checkUserPerms(mac->permissions, reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->id))
						WoL::sendMagicPacket(*wolHandler, mac->address);
					else {
						delete mac;
						return new HTTPResponse {403, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(FORBIDDEN))};
					}
					delete mac;
					auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
					*resultBody += FlashStorage<char>(PSTR("<h3>Magic Packet Invoked</h3><a href=\"/\">Back</a>"))();
					*resultBody += FlashStorage<char>::getString(HTML_END);
					return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
				} else if (param2 == FlashStorage<char>(PSTR("updatePerms=Update+Perms"))() && reinterpret_cast<EEPROMStorage::UserMetadata*>(request.data)->permissions == EEPROMStorage::UserPermissions::ADMIN) {
					auto mac = EEPROMStorage::getNearestMacAddress(id);
					auto splitPerms = Utilities::split(perms, "%7C");
					if (mac == nullptr || splitPerms->amount != mac->permissionsSize) {
						delete splitPerms;
						return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
					}
					int perm = 0;
					for (int i = 0; i < mac->permissionsSize; i++) {
						int buff = atoi(splitPerms->strings[i].c_str());
						mac->permissions[i] = buff;
						if (buff) perm++;
					}
					delete splitPerms;
					if (perm) {
						EEPROMStorage::removeNearestMacAddress(id);
						auto macId = mac->saveToEEPROM(id);
						delete mac;
						auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
						*resultBody += FlashStorage<char>(PSTR("<h3>Magic Packet Permissions Updated</h3><h4>ID: "))();
						*resultBody += String(macId);
						*resultBody += FlashStorage<char>(PSTR("</h4><a href=\"/\">Back</a>"))();
						*resultBody += FlashStorage<char>::getString(HTML_END);
						return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
					} else {
						delete mac;
						auto resultBody = new String(FlashStorage<char>::getString(HTML_BEGIN));
						*resultBody += FlashStorage<char>(PSTR("<h3>Can not set null permissions</h3><a href=\"/\">Back</a>"))();
						*resultBody += FlashStorage<char>::getString(HTML_END);
						return new HTTPResponse {200, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_HTML)}, 1, resultBody};
					}
				} else return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
			} else return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
		} else {
			delete params;
			return new HTTPResponse {500, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::TEXT_PLAIN)}, 1, new String(FlashStorage<char>::getString(INVALID_REQUEST))};
		}
	}};
}

HttpMiddleware* Middlewares::notFound404() {
	return new HttpMiddleware {HTTPMethods::ALL, FlashStorage<char>::getString(ASTERIX), [](HTTPRequest& request) -> HTTPResponse* {
		return new HTTPResponse {404, new String[1] {FlashStorage<char>::getString(CONTENT_TYPE::JSON)}, 1, new String(FlashStorage<char>(PSTR("{ \"Error\": 404 }"))())};
	}};
}
