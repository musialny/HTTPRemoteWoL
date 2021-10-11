/*
 * Utilities.h
 *
 * Created: 8/23/2021 10:53:07 PM
 *  Author: musialny
 */ 

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <WString.h>
#include <Arduino.h>
#include "ElasticArray.h"

namespace Utilities {
	struct SplittedString {
		int amount;
		String* strings;
		SplittedString();
		~SplittedString();
	};

	ElasticArray<int>* findAll(const String& value, const String& findingValue);
	SplittedString* split(const String& value, const String& splitter);
	// SplittedString* split(const String& value, int maxArraySize);
	String* decodeBASE64(const String& value, size_t inputShrink = 0);
	template<int SIZE>
	bool compareFixedSizeArray(const String& value, const char fixedSizeArray[]) {
		if (value.length() <= SIZE) {
			for (int i = 0; i < SIZE; i++) {
				if (value.charAt(i) != fixedSizeArray[i])
				return false;
			}
		} else return false;
		return true;
	}
	int calculateBitFieldsAllocation(int bits);
	bool checkUserPerms(const byte perms[], int userId);
	void setUserPerms(byte perms[], int userId, bool value = true);
	byte* parseUrlMacAddress(const String& address);
}

#endif /* UTILITIES_H_ */
