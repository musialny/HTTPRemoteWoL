#include <SPI.h>
#include <Ethernet.h>
#include "WoL.h"
#include "HTTPServer.h"

// WoL Config
const byte woLaddressesList[][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
const byte broadcastAddress[] = { 10, 10, 0, 255 };
WoLHandler* wol;

HTTPServer* httpServer;

void setup() {
	httpServer = new HTTPServer(new (byte[6]){0xC0, 0x06, 0x42, 0xC4, 0x40, 0x9D}, new IPAddress(10, 10, 0, 10), 80);
	wol = initWoLHandler(broadcastAddress);
}

void loop() {
	httpServer->listen([](){
		sendMagicPacket(*wol, woLaddressesList[0]);
	});
}
