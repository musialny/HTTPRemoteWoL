/* 
* Storage.cpp
*
* Created: 9/10/2021 11:21:26 AM
* Author: musialny
*/

#include <EEPROM.h>
#include "EEPROMStorage.h"
#include "FlashStorage.h"
#include "Utilities.h"

constexpr const int USER_DATA_ADDRESS = 0;

#define MAC_DATA_TABLE_AMOUNT_BEGIN (sizeof(byte) + (EEPROMStorage::getUsersAmount() * sizeof(EEPROMStorage::User)))
#define MAC_DATA_TABLE_ALLOCATION_SIZE ((2 * sizeof(byte)) + (EEPROMStorage::getUsersAmount() * sizeof(EEPROMStorage::User)))
#define MAC_DATA_TABLE_DATA_BEGIN(x) ((3 * sizeof(byte)) + (EEPROMStorage::getUsersAmount() * sizeof(EEPROMStorage::User)) + \
									 (x * (sizeof(EEPROMStorage::Mac::address) + sizeof(EEPROMStorage::Mac::name) + Utilities::calculateBitFieldsAllocation(EEPROMStorage::getUsersAmount()))))

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

EEPROMStorage::Mac::Mac(const byte address[], const char name[], const byte* permissions) {
	this->permissionsSize = Utilities::calculateBitFieldsAllocation(EEPROMStorage::getUsersAmount());
	this->permissions = new byte[this->permissionsSize];
	for (byte i = 0; i < sizeof(this->address); i++)
		this->address[i] = address[i];
	for (byte i = 0; i < this->permissionsSize; i++)
		this->permissions[i] = permissions[i];
	for (byte i = 0; i < sizeof(this->name); i++)
		this->name[i] = name[i];
}

EEPROMStorage::Mac::~Mac() {
	delete[] this->permissions;
}

int EEPROMStorage::Mac::saveToEEPROM(int slot, bool bumpCounter) {
	if (EEPROMStorage::getMacAddressesAmount() < EEPROM.read(MAC_DATA_TABLE_ALLOCATION_SIZE)) {
		auto save = [&bumpCounter](int i, byte* data, int permissionsSize) -> void {
			for (byte o = 0; o < sizeof(EEPROMStorage::Mac::address) + sizeof(EEPROMStorage::Mac::name) + permissionsSize; o++)
				EEPROM.write(MAC_DATA_TABLE_DATA_BEGIN(i) + o, data[o]);
			if (bumpCounter) EEPROM.write(MAC_DATA_TABLE_AMOUNT_BEGIN, EEPROMStorage::getMacAddressesAmount() + 1);
		};
		byte* data = reinterpret_cast<byte*>(malloc(sizeof(EEPROMStorage::Mac::address) + sizeof(EEPROMStorage::Mac::name) + this->permissionsSize));
		for (byte i = 0; i < this->permissionsSize; i++)
			data[i] = this->permissions[i];
		for (byte i = 0; i < sizeof(this->address); i++)
			data[i + this->permissionsSize] = this->address[i];
		for (byte i = 0; i < sizeof(this->name); i++)
			data[i + this->permissionsSize + sizeof(this->address)] = this->name[i];
		if (slot > -1) {
			save(slot, data, this->permissionsSize);
			delete[] data;
			return slot;
		} else if (!EEPROMStorage::getMacAddressesAmount()) {
			save(0, data, this->permissionsSize);
			delete[] data;
			return 0;
		} else {
			for (int i = 0; i < EEPROMStorage::getMacAddressesAmount(); i++) {
				if (!EEPROMStorage::isMacAddressExists(i)) {
					save(i, data, this->permissionsSize);
					delete[] data;
					return i;
				}
			}
			save(EEPROMStorage::getMacAddressesAmount(), data, this->permissionsSize);
			delete[] data;
			return EEPROMStorage::getMacAddressesAmount() - 1;
		}
		delete[] data;
	}
	return -1;
}

void EEPROMStorage::initStorage(byte userAmount, byte macAmount, const byte woLDefaultAddressList[][6], FlashStorage<char> woLDefaultAddressNames[], int woLDefaultAddressListAmount) {
	if (EEPROM.read(USER_DATA_ADDRESS) == 255) {
		EEPROM.write(USER_DATA_ADDRESS, userAmount);
		EEPROM.write(MAC_DATA_TABLE_ALLOCATION_SIZE, macAmount);
		EEPROM.write(MAC_DATA_TABLE_AMOUNT_BEGIN, 0);
		for (int i = 0; i < userAmount; i++)
			EEPROM.put(sizeof(byte) + (i * sizeof(EEPROMStorage::User)), !i ? EEPROMStorage::User(FlashStorage<char>(PSTR("admin"))(), FlashStorage<char>(PSTR("admin"))(),
					   EEPROMStorage::UserPermissions::ADMIN) : EEPROMStorage::User());
		auto perms = new byte[Utilities::calculateBitFieldsAllocation(EEPROMStorage::getUsersAmount())] {};
		perms[0] = 1;
		for (int i = 0; i < woLDefaultAddressListAmount; i++) {
			String& nameString = woLDefaultAddressNames[i].getString();
			char name[12];
			for (byte o = 0; o < sizeof(name); o++)
				name[o] = o >= nameString.length() ? '\0' : nameString.charAt(o);
			EEPROMStorage::Mac(woLDefaultAddressList[i], name, perms).saveToEEPROM();
		}
		delete[] perms;
	}
}

void EEPROMStorage::formatStorage() {
	for (int i = 0; i < 4096; i++) EEPROM.write(i, 255);
}

byte EEPROMStorage::getUsersAmount() {
	return EEPROM.read(USER_DATA_ADDRESS);
}

byte EEPROMStorage::getMacAddressesAmount() {
	return EEPROM.read(sizeof(byte) + (EEPROMStorage::getUsersAmount() * sizeof(EEPROMStorage::User)));
}

bool EEPROMStorage::isMacAddressExists(byte id) {
	if (id >= EEPROMStorage::getMacAddressesAmount()) return false;
	int address = MAC_DATA_TABLE_DATA_BEGIN(id);
	byte checksumSize = Utilities::calculateBitFieldsAllocation(EEPROMStorage::getUsersAmount());
	byte incrementator = 0;
	for (byte i = 0; i < checksumSize; i++)
		if (EEPROM.read(address + i) == 0) incrementator++;
	if (incrementator == checksumSize) return false;
	else return true;
}

byte EEPROMStorage::getNearestMacAddressId(byte id) {
	int offset = 0;
	for (int i = 0; i < id; i++) if (!EEPROMStorage::isMacAddressExists(i)) offset++;
	Serial.println("ID: " + String(id) + " | Offset: " + String(offset));
	/*for (int i = id; i < EEPROM.read(MAC_DATA_TABLE_ALLOCATION_SIZE); i++) {
		if (EEPROMStorage::isMacAddressExists(id + offset)) break;
		id++;
		if (i == EEPROM.read(MAC_DATA_TABLE_ALLOCATION_SIZE) - 1) return nullptr;
	}*/
}

void EEPROMStorage::removeNearestMacAddress(byte id) {
	if (EEPROMStorage::getMacAddressesAmount()) {
		for (int i = id; i < EEPROM.read(MAC_DATA_TABLE_ALLOCATION_SIZE); i++) {
			if (EEPROMStorage::isMacAddressExists(id)) break;
			id++;
			if (i == EEPROM.read(MAC_DATA_TABLE_ALLOCATION_SIZE) - 1) return;
		}
		int address = MAC_DATA_TABLE_DATA_BEGIN(id);
		for (byte i = 0; i < Utilities::calculateBitFieldsAllocation(EEPROMStorage::getUsersAmount()); i++)
			EEPROM.write(address + i, 0);
		byte buffer = EEPROMStorage::getMacAddressesAmount();
		EEPROM.write(MAC_DATA_TABLE_AMOUNT_BEGIN, !buffer ? 0 : buffer - 1);
	}
}

EEPROMStorage::Mac* EEPROMStorage::getNearestMacAddress(byte id) {
	if (id >= EEPROM.read(MAC_DATA_TABLE_ALLOCATION_SIZE)) return nullptr;
	EEPROMStorage::getNearestMacAddressId(id);
	return EEPROMStorage::getMacAddress(id);
}

EEPROMStorage::Mac* EEPROMStorage::getMacAddress(byte id) {
	if (!EEPROMStorage::isMacAddressExists(id)) return nullptr;
	int address = MAC_DATA_TABLE_DATA_BEGIN(id);
	int allocSize = Utilities::calculateBitFieldsAllocation(EEPROMStorage::getUsersAmount());
	auto permissions = new byte[allocSize];
	for (int i = 0; i < allocSize; i++)
		permissions[i] = EEPROM.read(address++);
	byte macAddress[sizeof(EEPROMStorage::Mac::address)] = {EEPROM.read(address++), EEPROM.read(address++), EEPROM.read(address++),
															EEPROM.read(address++), EEPROM.read(address++), EEPROM.read(address++)};
	char macName[sizeof(EEPROMStorage::Mac::name)];
	for (byte i = 0; i < sizeof(EEPROMStorage::Mac::name); i++)
		macName[i] = EEPROM.read(address++);
	auto result = new EEPROMStorage::Mac(macAddress, macName, permissions);
	delete[] permissions;
	return result;
}

EEPROMStorage::User* EEPROMStorage::getUserCredentials(byte userId) {
	auto user = new EEPROMStorage::User;
	EEPROM.get(!userId ? sizeof(byte) : sizeof(byte) + (userId * sizeof(EEPROMStorage::User)), *user);
	if (user->username[0] == '\0') {
		delete user;
		return nullptr;
	}
	return user;
}

EEPROMStorage::UserPermissions EEPROMStorage::getUserPermissions(byte userId) {
	return static_cast<EEPROMStorage::UserPermissions>(EEPROM.read((userId + 1) * sizeof(EEPROMStorage::User)));
}

bool EEPROMStorage::pushUser(const EEPROMStorage::User& user, int slot) {
	if (slot == -1) {	
		for (int i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
			if (EEPROM.read(sizeof(byte) + (i * sizeof(EEPROMStorage::User))) == '\0') {
				EEPROM.put(sizeof(byte) + (i * sizeof(EEPROMStorage::User)), user);
				return true;
			}
		}
		return false;
	} else {
		EEPROM.put(sizeof(byte) + (slot * sizeof(EEPROMStorage::User)), user);
		return true;
	}
}

bool EEPROMStorage::removeUser(byte id) {
	auto userPerms = EEPROMStorage::getUserPermissions(id);
	if (userPerms == UserPermissions::ADMIN) {
		byte admins = 0;
		for (int i = 0; i < EEPROMStorage::getUsersAmount(); i++) {
			userPerms = EEPROMStorage::getUserPermissions(i);
			if (userPerms == UserPermissions::ADMIN) admins++;
			if (admins > 1) break;
		}
		if (admins < 2) return false;
	}
	EEPROM.write(!id ? sizeof(byte) : sizeof(byte) + (id * sizeof(EEPROMStorage::User)), '\0');
	return true;
}
