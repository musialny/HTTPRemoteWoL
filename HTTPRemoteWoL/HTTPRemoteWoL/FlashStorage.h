/* 
* FlashStorage.h
*
* Created: 9/12/2021 12:56:32 AM
* Author: musialny
*/


#ifndef __FLASHSTORAGE_H__
#define __FLASHSTORAGE_H__

#include <WString.h>

template<typename T>
class FlashStorage {
private:
	const T* flashMemoryPointer;
	
public:
	FlashStorage(const T* flashMemoryPointer) : flashMemoryPointer(flashMemoryPointer) {};
	~FlashStorage() = default;
	
	T operator() () {
		return pgm_read_byte(this->flashMemoryPointer);
	}
};

template<>
class FlashStorage<char> {
private:
	const char* flashMemoryPointer;
	String result;
	
public:
	FlashStorage(const char* flashMemoryPointer) : flashMemoryPointer(flashMemoryPointer) {};
	~FlashStorage() = default;
	
	String& operator() () {
		for (int i = 0; pgm_read_byte(this->flashMemoryPointer + i); i++)
			this->result += static_cast<char>(pgm_read_byte(this->flashMemoryPointer + i));
		return this->result;
	}
};

#endif //__FLASHSTORAGE_H__
