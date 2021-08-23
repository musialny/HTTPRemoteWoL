/*
 * HTTPServer.h
 *
 * Created: 8/23/2021 12:45:38 AM
 *  Author: musialny
 */ 


#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <SPI.h>
#include <Ethernet.h>

class HTTPServer
{
private:
	String (*middlewares[])(String request);
	EthernetServer* server;
public:
	HTTPServer(byte* deviceMacAddress, IPAddress* ip, int port = 80);
	~HTTPServer();
	
	void listen(void (*middleware)());
};


#endif /* HTTPSERVER_H_ */