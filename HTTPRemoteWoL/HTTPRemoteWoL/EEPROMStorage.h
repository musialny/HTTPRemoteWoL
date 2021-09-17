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
	
	void initStorage(byte userAmount);
	byte readRawStorage(int address);
	byte getUsersAmount();
	EEPROMStorage::User* getUserCredentials(byte userId);
	bool pushUser(const EEPROMStorage::User& user);
}

#endif //__STORAGE_H__
