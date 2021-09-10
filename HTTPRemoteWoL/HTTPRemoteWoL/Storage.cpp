/* 
* Storage.cpp
*
* Created: 9/10/2021 11:21:26 AM
* Author: musialny
*/

#include <EEPROM.h>
#include "Storage.h"

constexpr int USER_DATA_ADDRESS = 0;

Storage::User::User(String username, String password, UserPermissions permissions) : permissions(permissions) {
	for (int i = 0; i < sizeof(this->username); i++) {
		if (i >= username.length()) this->username[i] = '\0';
		this->username[i] = username.charAt(i);
	}
	for (int i = 0; i < sizeof(this->password); i++) {
		if (i >= password.length()) this->password[i] = '\0';
		this->password[i] = password.charAt(i);
	}
}

void Storage::initStorage(byte userAmount) {
	if (EEPROM.read(USER_DATA_ADDRESS) == 255) {
		EEPROM.write(USER_DATA_ADDRESS, userAmount);
		for (int i = 0; i < userAmount; i++)
			EEPROM.put(sizeof(byte) + (i * sizeof(Storage::User)), !i ? Storage::User("admin", "adminadmin", Storage::UserPermissions::ADMIN) : Storage::User());
	}
}

byte Storage::readRawStorage(int address) {
	return EEPROM.read(address);
}

byte Storage::getUsersAmount() {
	return EEPROM.read(USER_DATA_ADDRESS);
}

Storage::User* Storage::getUserCredentials(byte userId) {
	auto user = new Storage::User;
	EEPROM.get(!userId ? sizeof(byte) : sizeof(byte) + (userId * sizeof(Storage::User)), *user);
	return user;
}
