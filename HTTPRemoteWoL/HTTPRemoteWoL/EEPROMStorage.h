/* 
* Storage.h
*
* Created: 9/10/2021 11:21:27 AM
* Author: musialny
*/

#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <Arduino.h>
#include <WString.h>
#include "FlashStorage.h"

namespace EEPROMStorage {
	enum class UserPermissions: byte {
		USER,
		ADMIN
	};	

	struct User {
		char username[8];
		char password[12];
		UserPermissions permissions;

		User(String username = "", String password = "", EEPROMStorage::UserPermissions permissions = EEPROMStorage::UserPermissions::USER);
	};
	
	struct UserMetadata {
		byte id;
		char username[9];
		UserPermissions permissions;

		UserMetadata(byte id, const char username[], EEPROMStorage::UserPermissions permissions);
	};
	
	struct Mac {
		char name[12];
		byte address[6];
		byte* permissions;
		int permissionsSize;

		Mac(const byte address[6], const char name[12], const byte* permissions);
		~Mac();
		int saveToEEPROM(int slot = -1, bool bumpCounter = true);
	};
	
	void initStorage(byte userAmount, byte macAmount, const byte woLDefaultAddressList[][6], FlashStorage<char> woLDefaultAddressNames[], int woLDefaultAddressListAmount);
	void formatStorage();
	byte getUsersAmount();
	byte getMacAddressesAmount();
	bool isMacAddressExists(byte id);
	byte getNearestMacAddressId(byte id);
	void removeNearestMacAddress(byte id);
	EEPROMStorage::Mac* getNearestMacAddress(byte id);
	EEPROMStorage::Mac* getMacAddress(byte id);
	EEPROMStorage::User* getUserCredentials(byte userId);
	EEPROMStorage::UserPermissions getUserPermissions(byte userId);
	bool pushUser(const EEPROMStorage::User& user, int slot = -1);
	bool removeUser(byte id);
}

#endif //__STORAGE_H__
