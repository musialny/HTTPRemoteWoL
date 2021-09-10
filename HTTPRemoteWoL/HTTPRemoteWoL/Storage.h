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

namespace Storage {
	enum class UserPermissions: byte {
		ADMIN,
		USER
	};	

	struct User {
		char username[8];
		char password[12];
		UserPermissions permissions;

		User(String username = "", String password = "", Storage::UserPermissions permissions = Storage::UserPermissions::USER);
	};
	
	void initStorage(byte userAmount);
	byte readRawStorage(int address);
}

#endif //__STORAGE_H__
