#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPServer.h"
#include "Middlewares.h"
#include "WoL.h"
#include "EEPROMStorage.h"
#include "FlashStorage.h"
#include "Utilities.h"
// #include <EEPROM.h>

constexpr const int woLDefaultAddressListAmount = 1;
constexpr const byte woLDefaultAddressList[woLDefaultAddressListAmount][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};

WoLHandler* wolHandler;
HTTPServer* httpServer;

void setup() {
	/*Serial.begin(9600); 
	Serial.println(FlashStorage<char>(PSTR("[Serial Port Inited]\n"))());*/
	
	// for (int i = 0; i < 1024; i++) EEPROM.write(i, 255);
	EEPROMStorage::initStorage(10, woLDefaultAddressList, woLDefaultAddressListAmount);
	
	/*for (int i = 0; i < 1024; i++) {
		char result = EEPROMStorage::readRawStorage(i);
		// if (result != 255)
			Serial.print(String(static_cast<byte>(result)) + "|");
	}
	Serial.print("\r");
	for (int i = 0; i < 1024; i++) {
		char result = EEPROMStorage::readRawStorage(i);
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
		Serial.print(FlashStorage<char>(PSTR("user->username = "))() + String(user->username));
		Serial.print(FlashStorage<char>(PSTR(" | user->password = "))() + String(user->password));
		Serial.print(FlashStorage<char>(PSTR(" | user->permissions = "))() + String(static_cast<byte>(user->permissions)));
		Serial.println();
		delete user;
	}*/
	
	const byte deviceMac[6] = {0xC0, 0x06, 0x42, 0xC4, 0x40, 0x9D};
	const byte broadcastAddress[4] = { 10, 10, 0, 255 };
	httpServer = new HTTPServer(deviceMac, IPAddress(10, 10, 0, 10), 80);
	httpServer->use(Middlewares::auth())
		.use(Middlewares::homePage())
		.use(Middlewares::users())
		.use(Middlewares::wol())
		.use(Middlewares::notFound404());
	wolHandler = new WoLHandler(broadcastAddress);
}

void loop() {
	httpServer->listen();
}

// TODO: Avoid user username duplication
// TODO: Allow to change password
// TODO: Create request limiters
