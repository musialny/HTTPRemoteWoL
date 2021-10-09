/*
 * WoL.cpp
 *
 * Created: 8/23/2021 12:51:43 AM
 *  Author: musialny
 */

#include "WoL.h"

WoL::WoLHandler::WoLHandler(const IPAddress& ip) {
	this->udp.begin(7);
	this->broadCastIp = new byte[4] {ip[0], ip[1], ip[2], ip[3]};
}

WoL::WoLHandler::~WoLHandler() {
	delete this->broadCastIp;
}

void WoL::sendMagicPacket(WoLHandler& woLHandler, const byte* const remote_MAC_ADD) {
	int wolPort = 9;
	byte magicPacket[102];

	int magicPacketIterator = 6;
	for (int i = 0; i < 6; i++) magicPacket[i] = 0xFF;
	for (int i = 0; i < 16; i++) for (int o = 0; o < 6; o++) magicPacket[magicPacketIterator++] = remote_MAC_ADD[o];
	
	woLHandler.udp.beginPacket(woLHandler.broadCastIp, wolPort);
	woLHandler.udp.write(magicPacket, sizeof magicPacket);
	woLHandler.udp.endPacket();
}
