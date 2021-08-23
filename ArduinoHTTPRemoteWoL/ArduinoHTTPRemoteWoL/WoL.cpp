/*
 * WoL.cpp
 *
 * Created: 8/23/2021 12:51:43 AM
 *  Author: musialny
 */

#include "WoL.h"

WoLHandler* initWoLHandler(const byte* const broadCastIp) {
	auto woLHandler = new WoLHandler;
	woLHandler->udp.begin(7);
	woLHandler->broadCastIp = const_cast<byte*>(broadCastIp);
	return woLHandler;
}

void sendMagicPacket(WoLHandler& woLHandler, const byte* const remote_MAC_ADD)
{
	int wolPort = 9;
	byte magicPacket[102];
	
	int i = 0;
	int o = 0;
	int magicPacketIterator = 6;
	for (i = 0; i < 6; i++) magicPacket[i] = 0xFF;
	for (i = 0; i < 16; i++) for (o = 0; o < 6; o++) magicPacket[magicPacketIterator++] = remote_MAC_ADD[o];
	
	woLHandler.udp.beginPacket( woLHandler.broadCastIp, wolPort);
	woLHandler.udp.write(magicPacket, sizeof magicPacket);
	woLHandler.udp.endPacket();
}