#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPServer.h"
#include "Middlewares.h"
#include "WoL.h"
#include "EEPROMStorage.h"
#include "FlashStorage.h"
#include "Utilities.h"

constexpr int FACTORY_RESET_PIN = 8; // Push high state
constexpr int STATUS_PIN = 9;

constexpr const int woLDefaultAddressListAmount = 1;
constexpr const byte woLDefaultAddressList[woLDefaultAddressListAmount][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
namespace woLDefaultAddressNames {
	const char PC[] PROGMEM = "PC";
}

WoL::WoLHandler* wolHandler;
HTTPServer* httpServer;

void setup() {
	pinMode(FACTORY_RESET_PIN, INPUT);
	if (digitalRead(FACTORY_RESET_PIN)) EEPROMStorage::formatStorage();
	
	FlashStorage<char> woLNames[woLDefaultAddressListAmount] = {woLDefaultAddressNames::PC};
	EEPROMStorage::initStorage(20, 100, woLDefaultAddressList, woLNames, woLDefaultAddressListAmount);
	
	const byte deviceMac[6] = {0xC0, 0x06, 0x42, 0xC4, 0x40, 0x9D};
	const byte broadcastAddress[4] = { 10, 10, 0, 255 };
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
