#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPServer.h"
#include "Middlewares.h"
#include "WoL.h"

WoLHandler* wol;
HTTPServer* httpServer;

void setup() {
	/*Serial.begin(9600);
	Serial.println("[Serial Port Started]\n");*/

	httpServer = new HTTPServer(new (byte[6]){0xC0, 0x06, 0x42, 0xC4, 0x40, 0x9D}, new IPAddress(10, 10, 0, 10), 80);
	httpServer->use(Middlewares::auth()).use(Middlewares::homePage()).use(Middlewares::homePage(HTTPMethods::POST)).use(Middlewares::subPage());
	const byte broadcastAddress[] = { 10, 10, 0, 255 };
	wol = new WoLHandler(broadcastAddress);
}

void loop() {
	httpServer->listen();
}
