#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPServer.h"
#include "Middlewares.h"
#include "WoL.h"
#include "EEPROMStorage.h"
#include "FlashStorage.h"

constexpr int FACTORY_RESET_PIN = 8; // Push high state
constexpr int STATUS_PIN = 9;

namespace woLDefaultAddressNames {
	const char PC[] PROGMEM = "PC";
}

WoL::WoLHandler* wolHandler;
HTTPServer* httpServer;

void setup() {
	constexpr const int woLDefaultAddressListAmount = 1;
	constexpr const byte woLDefaultAddressList[woLDefaultAddressListAmount][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
	FlashStorage<char> woLNames[woLDefaultAddressListAmount] = {woLDefaultAddressNames::PC};
	constexpr const byte deviceMac[6] = {0xD0, 0xD4, 0xC3, 0x71, 0x82, 0x19};
	constexpr const byte broadcastAddress[4] = { 10, 10, 0, 255 };
	
	pinMode(FACTORY_RESET_PIN, INPUT);
	if (digitalRead(FACTORY_RESET_PIN)) EEPROMStorage::formatStorage();
	
	EEPROMStorage::initStorage(20, 100, woLDefaultAddressList, woLNames, woLDefaultAddressListAmount);
	httpServer = new HTTPServer(deviceMac, IPAddress(10, 10, 0, 10), 80, STATUS_PIN);
	httpServer->use(Middlewares::auth())
		.use(Middlewares::homePage())
		.use(Middlewares::users())
		.use(Middlewares::wol())
		.use(Middlewares::notFound404());
	wolHandler = new WoL::WoLHandler(broadcastAddress);
}

void loop() {
	httpServer->listen();
}

// TODO: Avoid user username duplication
// TODO: Allow to change password
// TODO: Create request limiters
