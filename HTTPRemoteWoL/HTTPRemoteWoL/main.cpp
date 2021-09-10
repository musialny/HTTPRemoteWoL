#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPServer.h"
#include "Middlewares.h"
#include "WoL.h"
#include "Storage.h"
#include <EEPROM.h>

WoLHandler* wol;
HTTPServer* httpServer;

void setup() {
	Serial.begin(9600);
	Serial.println("[Serial Port Started]\n");
	
	// for (int i = 0; i < 1024; i++) EEPROM.write(i, 255);
	Storage::initStorage(5);
	
	for (int i = 0; i < 1024; i++) {
		char result = Storage::readRawStorage(i);
		if (result != 255)
			Serial.print(String(static_cast<byte>(result)) + "|");
	}
	Serial.println();
	for (int i = 0; i < 1024; i++) {
		char result = Storage::readRawStorage(i);
		if (result != 255)
			Serial.print(String(result) + [&result]() -> String {
				String returnable;
				if (result == 0) return " ";
				for (int i = 0; i < String(static_cast<byte>(result)).length() - 1; i++) returnable += " ";
				return returnable;
			}() + "|");
	}
	Serial.println();
	for (int i = 0; i < Storage::getUsersAmount(); i++) {
		auto user = Storage::getUserCredentials(i);
		Serial.print("user->username = " + String(user->username));
		Serial.print(" | user->password = " + String(user->password));
		Serial.print(" | user->permissions = " + String(static_cast<byte>(user->permissions)));
		Serial.println();
		delete user;
	}

	httpServer = new HTTPServer(new (byte[6]){0xC0, 0x06, 0x42, 0xC4, 0x40, 0x9D}, new IPAddress(10, 10, 0, 10), 80);
	httpServer->use(Middlewares::auth()).use(Middlewares::homePage()).use(Middlewares::homePage(HTTPMethods::POST)).use(Middlewares::subPage());
	const byte broadcastAddress[] = { 10, 10, 0, 255 };
	wol = new WoLHandler(broadcastAddress);
}

void loop() {
	httpServer->listen();
}
