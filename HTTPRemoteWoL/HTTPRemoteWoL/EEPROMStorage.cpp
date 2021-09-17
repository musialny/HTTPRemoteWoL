/* 
* Storage.cpp
*
* Created: 9/10/2021 11:21:26 AM
* Author: musialny
*/

#include <EEPROM.h>
#include "EEPROMStorage.h"
#include "FlashStorage.h"

constexpr int USER_DATA_ADDRESS = 0;

EEPROMStorage::User::User(String username, String password, UserPermissions permissions) : permissions(permissions) {
	for (int i = 0; i < sizeof(this->username); i++) {
		if (i >= username.length()) this->username[i] = '\0';
		this->username[i] = username.charAt(i);
	}
	for (int i = 0; i < sizeof(this->password); i++) {
		if (i >= password.length()) this->password[i] = '\0';
		this->password[i] = password.charAt(i);
	}
}

EEPROMStorage::UserMetadata::UserMetadata(byte id, const char username[], EEPROMStorage::UserPermissions permissions) : id(id), permissions(permissions) {
	memcpy(this->username, username, sizeof(EEPROMStorage::User::username));
	this->username[sizeof(this->username) - 1] = '\0';
}

void EEPROMStorage::initStorage(byte userAmount) {
	if (EEPROM.read(USER_DATA_ADDRESS) == 255) {
		EEPROM.write(USER_DATA_ADDRESS, userAmount);
		for (int i = 0; i < userAmount; i++)
			EEPROM.put(sizeof(byte) + (i * sizeof(EEPROMStorage::User)), !i ? EEPROMStorage::User(FlashStorage<char>(PSTR("admin"))(), FlashStorage<char>(PSTR("adminadmin"))(), EEPROMStorage::UserPermissions::ADMIN) : EEPROMStorage::User());
	}
}

byte EEPROMStorage::readRawStorage(int address) {
	return EEPROM.read(address);
}

byte EEPROMStorage::getUsersAmount() {
	return EEPROM.read(USER_DATA_ADDRESS);
}

EEPROMStorage::User* EEPROMStorage::getUserCredentials(byte userId) {
	auto user = new EEPROMStorage::User;
	EEPROM.get(!userId ? sizeof(byte) : sizeof(byte) + (userId * sizeof(EEPROMStorage::User)), *user);
	return user;
}

bool EEPROMStorage::pushUser(const EEPROMStorage::User& user) {
	for (int i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
		if (EEPROM.read(sizeof(byte) + (i * sizeof(EEPROMStorage::User))) == '\0') {
			EEPROM.put(sizeof(byte) + (i * sizeof(EEPROMStorage::User)), user);
			return true;
		}
	}
	return false;
}