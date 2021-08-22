#include <SPI.h>
#include <Ethernet.h>

#define ENABLE_SERIAL

byte deviceMacAddress[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(10, 10, 0, 10);

EthernetServer server(80);

void setup() {
	#ifdef ENABLE_SERIAL
		Serial.begin(9600);
		while (!Serial) {}
		Serial.println("Ethernet WebServer OwO Example");
	#endif // ENABLE_SERIAL

	Ethernet.begin(deviceMacAddress, ip);
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		#ifdef ENABLE_SERIAL
			Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
		#endif // ENABLE_SERIAL
		while (true) {
			delay(1);
		}
	}
	if (Ethernet.linkStatus() == LinkOFF) {
		#ifdef ENABLE_SERIAL
			Serial.println("Ethernet cable is not connected.");
		#endif // ENABLE_SERIAL
	}

	server.begin();
	#ifdef ENABLE_SERIAL
		Serial.print("server is at ");
		Serial.println(Ethernet.localIP());
	#endif // ENABLE_SERIAL
}


void loop() {
	EthernetClient client = server.available();
	if (client) {
		#ifdef ENABLE_SERIAL
			Serial.println("new client");
		#endif // ENABLE_SERIAL
		
		boolean currentLineIsBlank = true;
		while (client.connected()) {
		if (client.available()) {
			char c = client.read();
			#ifdef ENABLE_SERIAL
				Serial.write(c);
			#endif // ENABLE_SERIAL
			// if you've gotten to the end of the line (received a newline
			// character) and the line is blank, the http request has ended,
			// so you can send a reply
			if (c == '\n' && currentLineIsBlank) {
				// send a standard http response header
				client.println("HTTP/1.1 200 OK");
				client.println("Content-Type: text/html");
				client.println("Connection: close");  // the connection will be closed after completion of the response
				client.println("Refresh: 5");  // refresh the page automatically every 5 sec
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
		#ifdef ENABLE_SERIAL
			Serial.println("client disconnected");
		#endif // ENABLE_SERIAL
	}
}
