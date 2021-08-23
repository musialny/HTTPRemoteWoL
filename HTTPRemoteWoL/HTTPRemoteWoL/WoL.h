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
};

WoLHandler* initWoLHandler(const byte* const broadCastIp);
void sendMagicPacket(WoLHandler& woLHandler, const byte* const remote_MAC_ADD);


#endif /* WOL_H_ */
