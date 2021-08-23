/*
 * WoL.cpp
 *
 * Created: 8/23/2021 12:51:43 AM
 *  Author: musialny
 */

#include "WoL.h"

WoLHandler* initWoLHandler(byte* broadCastIp, byte* remote_MAC_ADD) {
	auto woLHandler = new WoLHandler;
	woLHandler->udp.begin(7);
	woLHandler->broadCastIp = broadCastIp;
	woLHandler->remote_MAC_ADD = remote_MAC_ADD;
	return woLHandler;
}

void sendMagicPacket(WoLHandler& woLHandler)
{
	int wolPort = 9;
	
	byte magicPacket[102];
	int Ciclo = 0, CicloMacAdd = 0, IndiceArray = 0;
	
	for( Ciclo = 0; Ciclo < 6; Ciclo++)
	{
		magicPacket[IndiceArray] = 0xFF;
		IndiceArray++;
	}
	
	for( Ciclo = 0; Ciclo < 16; Ciclo++ )
	{
		for( CicloMacAdd = 0; CicloMacAdd < 6; CicloMacAdd++)
		{
			magicPacket[IndiceArray] = woLHandler.remote_MAC_ADD[CicloMacAdd];
			IndiceArray++;
		}
	}
	
	woLHandler.udp.beginPacket( woLHandler.broadCastIp, wolPort);
	woLHandler.udp.write(magicPacket, sizeof magicPacket);
	woLHandler.udp.endPacket();

}