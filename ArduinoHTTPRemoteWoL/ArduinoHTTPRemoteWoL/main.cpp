#include <SPI.h>
#include <Ethernet.h>

#include "WoL.h"

byte deviceMacAddress[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte woLaddressesList[][6] = {{ 0x18, 0xC0, 0x4D, 0x85, 0x10, 0x2F }};
byte broadcastAddress[] = { 10, 10, 0, 255 };
IPAddress ip(10, 10, 0, 10);

WoLHandler* wol;

EthernetServer server(80);

void setup() {
	pinMode(9, OUTPUT);
	
	Ethernet.begin(deviceMacAddress, ip);
	
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		while (true) {
			digitalWrite(9, HIGH);
		}
	}
	boolean statusLedState = false;
	while(Ethernet.linkStatus() == LinkOFF) {
		if (!statusLedState) {
			digitalWrite(9, HIGH);
			statusLedState = true;
			delay(500);
			} else {
			digitalWrite(9, LOW);
			statusLedState = false;
			delay(500);
		}
	}
	digitalWrite(9, LOW);
	
	wol = initWoLHandler(broadcastAddress, woLaddressesList[0]);
	server.begin();
}


void loop() {
	EthernetClient client = server.available();
	if (client) {
		boolean currentLineIsBlank = true;
		while (client.connected()) {
		if (client.available()) {
			char c = client.read();
			// if you've gotten to the end of the line (received a newline
			// character) and the line is blank, the http request has ended,
			// so you can send a reply
			if (c == '\n' && currentLineIsBlank) {
				// send a standard http response header
				client.println("HTTP/1.1 200 OK");
				client.println("Content-Type: text/html");
				client.println("Connection: close");  // the connection will be closed after completion of the response
				client.println();
				client.println("<!DOCTYPE HTML>");
				client.println("<html>");
				// output the value of each analog input pin
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
				if (c == '\n') {
				// you're starting a new line
					currentLineIsBlank = true;
				} else if (c != '\r') {
				// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
		client.stop();
		sendMagicPacket(*wol);
	}
}
