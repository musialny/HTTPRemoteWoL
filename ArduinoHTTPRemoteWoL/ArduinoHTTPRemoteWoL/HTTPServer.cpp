/*
 * HTTPServer.cpp
 *
 * Created: 8/23/2021 12:44:59 AM
 *  Author: musialny
 */ 

#include "HTTPServer.h"

constexpr int STATUS_PIN = 9; 

HTTPServer::HTTPServer(byte* deviceMacAddress, IPAddress* ip, int port) {
	this->server = new EthernetServer(port);
	Ethernet.begin(deviceMacAddress, *ip);
	
	pinMode(STATUS_PIN, OUTPUT);
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		while (true) {
			digitalWrite(STATUS_PIN, HIGH);
		}
	}
	boolean statusLedState = false;
	while(Ethernet.linkStatus() == LinkOFF) {
		if (!statusLedState) {
			digitalWrite(STATUS_PIN, HIGH);
			statusLedState = true;
			delay(500);
			} else {
			digitalWrite(STATUS_PIN, LOW);
			statusLedState = false;
			delay(500);
		}
	}
	digitalWrite(STATUS_PIN, LOW);
	
	this->server->begin();
}

HTTPServer::~HTTPServer() {
	delete this->server;
}

void HTTPServer::listen(void (*middleware)()) {
	EthernetClient client = server->available();
	if (client) {
		boolean currentLineIsBlank = true;
		while (client.connected()) {
		if (client.available()) {
			char c = client.read();
			if (c == '\n' && currentLineIsBlank) {
				client.println("HTTP/1.1 200 OK");
				client.println("Content-Type: text/html");
				client.println("Connection: close");
				client.println();
				client.println("<!DOCTYPE HTML>");
				client.println("<html>");
				for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
					int sensorReading = analogRead(analogChannel);
					client.print("analog input ");
					client.print(analogChannel);
					client.print(" is ");
					client.print(sensorReading);
					client.println("<br />");
				}
				client.println("</html>");
				break;
			}
				if (c == '\n') currentLineIsBlank = true;
				else if (c != '\r') currentLineIsBlank = false;
			}
		}
		delay(1);
		client.stop();
		middleware();
	}
}