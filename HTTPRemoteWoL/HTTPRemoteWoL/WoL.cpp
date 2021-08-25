/*
 * WoL.cpp
 *
 * Created: 8/23/2021 12:51:43 AM
 *  Author: musialny
 */

#include "WoL.h"

WoLHandler::WoLHandler(const byte* const broadCastIp) {
	this->udp.begin(7);
	this->broadCastIp = new byte[4] {broadCastIp[0], broadCastIp[1], broadCastIp[2], broadCastIp[3]};
}

WoLHandler::~WoLHandler() {
	delete this->broadCastIp;
}

void sendMagicPacket(WoLHandler& woLHandler, const byte* const remote_MAC_ADD) {
	int wolPort = 9;
	byte magicPacket[102];

	int magicPacketIterator = 6;
	for (int i = 0; i < 6; i++) magicPacket[i] = 0xFF;
	for (int i = 0; i < 16; i++) for (int o = 0; o < 6; o++) magicPacket[magicPacketIterator++] = remote_MAC_ADD[o];
	
	woLHandler.udp.beginPacket(woLHandler.broadCastIp, wolPort);
	woLHandler.udp.write(magicPacket, sizeof magicPacket);
	woLHandler.udp.endPacket();
}
