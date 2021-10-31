﻿#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPServer.h"
#include "Middlewares.h"
#include "WoL.h"
#include "EEPROMStorage.h"
#include "FlashStorage.h"

#include <EEPROM.h>

constexpr int FACTORY_RESET_PIN = 8; // Push high state
constexpr int STATUS_PIN = 9;

/*namespace woLDefaultAddressNames {
	const char PC[] PROGMEM = "PC";
}*/

WoL::WoLHandler* wolHandler;
HTTPServer* httpServer;

void setup() {
	Serial.begin(9600);
	Serial.println(FlashStorage<char>(PSTR("[ Serial Inited ]"))());
	for (int i = 0; i < 4 * 1024; i++) {
		char result = EEPROM.read(i);
		// if (result != 255)
			Serial.print(String(static_cast<byte>(result)) + "|");
	}
	Serial.println();
	for (int i = 0; i < 4 * 1024; i++) {
		char result = EEPROM.read(i);
		if (result != 255)
			Serial.print(String(result) + [&result]() -> String {
				String returnable;
				if (result == 0) return " ";
				else if (result == 10 || result == 255) return "  ";
				else for (int i = 0; i < String(static_cast<byte>(result)).length() - 1; i++) returnable += " ";
				return returnable;
			}() + "|");
	}
	Serial.println();
	for (int i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
		auto user = EEPROMStorage::getUserCredentials(i);
		if (user == nullptr) continue;
		Serial.print(FlashStorage<char>(PSTR("user->username = "))() + String(user->username));
		Serial.print(FlashStorage<char>(PSTR(" | user->password = "))() + String(user->password));
		Serial.print(FlashStorage<char>(PSTR(" | user->permissions = "))() + String(static_cast<byte>(user->permissions)));
		Serial.println();
		delete user;
	}
	
	Serial.println("------Is exists------");
	for (int i = 0; i < 150; i++)
		Serial.println(String(i) + " | " + String(EEPROMStorage::isMacAddressExists(i)));
	Serial.println("---------------------");
	
	/*constexpr const int woLDefaultAddressListAmount = 1;
	constexpr const byte woLDefaultAddressList[woLDefaultAddressListAmount][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
	FlashStorage<char> woLDefaultAddressListNames[woLDefaultAddressListAmount] = {woLDefaultAddressNames::PC};*/
	constexpr const byte deviceMac[6] = {0xD0, 0xD4, 0xC3, 0x71, 0x82, 0x19};
	
	pinMode(FACTORY_RESET_PIN, INPUT);
	if (digitalRead(FACTORY_RESET_PIN)) EEPROMStorage::formatStorage();
	
	EEPROMStorage::initStorage(20, 150, nullptr, nullptr, 0/*woLDefaultAddressList, woLDefaultAddressListNames, woLDefaultAddressListAmount*/);
	httpServer = new HTTPServer(deviceMac, IPAddress(10, 10, 0, 10), true, 80, STATUS_PIN);
	httpServer->use(Middlewares::auth())
		.use(Middlewares::homePage())
		.use(Middlewares::users())
		.use(Middlewares::user())
		.use(Middlewares::wol())
		.use(Middlewares::notFound404());
	wolHandler = new WoL::WoLHandler(IPAddress (~Ethernet.subnetMask()[0] + (Ethernet.localIP()[0] & Ethernet.subnetMask()[0]), ~Ethernet.subnetMask()[1] + (Ethernet.localIP()[1] & Ethernet.subnetMask()[1]),
												~Ethernet.subnetMask()[2] + (Ethernet.localIP()[2] & Ethernet.subnetMask()[2]), ~Ethernet.subnetMask()[3] + (Ethernet.localIP()[3] & Ethernet.subnetMask()[3])));
}

void loop() {
	httpServer->listen<512, 512, 512>();
}
