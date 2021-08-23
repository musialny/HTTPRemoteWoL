/*
 * WoL.h
 *
 * Created: 8/23/2021 12:52:16 AM
 *  Author: musialny
 */ 

#include <SPI.h>
#include <Ethernet.h>

#ifndef WOL_H_
#define WOL_H_

struct WoLHandler {
	EthernetUDP udp;
	byte* broadCastIp;
	byte* remote_MAC_ADD;
};

WoLHandler* initWoLHandler(byte* broadCastIp, byte* remote_MAC_ADD);
void sendMagicPacket(WoLHandler& woLHandler);


#endif /* WOL_H_ */