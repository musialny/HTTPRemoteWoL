#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPServer.h"
#include "Middlewares.h"
#include "WoL.h"

// WoL Config
const byte woLaddressesList[][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
const byte broadcastAddress[] = { 10, 10, 0, 255 };
WoLHandler* wol;

HTTPServer* httpServer;

void setup() {
	Serial.begin(9600);
	while (!Serial) {}
	
	httpServer = new HTTPServer(new (byte[6]){0xC0, 0x06, 0x42, 0xC4, 0x40, 0x9D}, new IPAddress(10, 10, 0, 10), 80);
	httpServer->use(Middlewares::getMiddlewares());
	wol = initWoLHandler(broadcastAddress);
}

void loop() {
	httpServer->listen([](){
		sendMagicPacket(*wol, woLaddressesList[0]);
	});
}
	