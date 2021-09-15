/*
 * Utilities.cpp
 *
 * Created: 8/23/2021 10:52:58 PM
 *  Author: musialny
 */ 

#include "Utilities.h"

Utilities::SplittedString::SplittedString() {
	this->amount = 0;
	this->strings = nullptr;
}

Utilities::SplittedString::~SplittedString() {
	delete[] this->strings;
}

ElasticArray<int>* Utilities::findAll(const String& value, const String& findingValue) {
	auto result = new ElasticArray<int>;
	for (int i = 0; i < value.length(); i++) {
		int subIndex = i;
		bool isEqual = true;
		for (int o = 0; o < findingValue.length(); o++) {
			if (value.charAt(subIndex++) != findingValue.charAt(o)) {
				isEqual = false;
				break;
			}
		}
		if (isEqual) result->push(i);
	}
	return result;
}

Utilities::SplittedString* Utilities::split(const String& value, const String& splitter) {
	auto result = new Utilities::SplittedString;
	auto splitPoints = findAll(value, splitter);
	if (splitPoints->length()) {
		result->amount = splitPoints->length() + 1;
		result->strings = new String[result->amount];
		int index = 0;
		for (int i = 0; i < value.length(); i++) {
			if (i == (*splitPoints)[index < splitPoints->length() ? index : 0]) {
				i += splitter.length();
				if (i >= value.length()) break;
				index++;
			}
			result->strings[index] += value.charAt(i);
		}
	}
	delete splitPoints;
	return result;
}

Utilities::SplittedString* Utilities::split(const String& value, int maxStringSize) {
	auto result = new Utilities::SplittedString;
	result->amount = value.length() / maxStringSize + (value.length() % maxStringSize ? 1 : 0);
	result->strings = new String[result->amount];
	int resultAmountCounter = 0;
	for (int i = 0; i < value.length(); i++) {
		if (i == maxStringSize * (resultAmountCounter + 1)) resultAmountCounter++;
		result->strings[resultAmountCounter] += value.charAt(i);
	}
	return result;
}

String* Utilities::decodeBASE64(const String& value, size_t inputShrink) {
	auto result = new String;
	auto encode = [](char value) -> byte {
		if (value >= 'A' && value <= 'Z')
			return value - 65;
		else if (value >= 'a' && value <= 'z')
			return value - 71;
		else if (value >= '0' && value <= '9')
			return value + 4;
		else if (value == '+')
			return 62;
		else if (value == '/')
			return 63;
		return 0;
	};
	for (int i = 0; i < value.length() - inputShrink;) {
		if (i >= value.length() - inputShrink) break;
		auto chunk = new char[4] {};
		int iBuffer[4] = {i++, i++, i++, i++};
		chunk[0] = (encode(value.charAt(iBuffer[0])) << 2) | (encode(value.charAt(iBuffer[1])) >> 4);
		chunk[1] = (encode(value.charAt(iBuffer[1])) << 4) | (encode(value.charAt(iBuffer[2])) >> 2);
		chunk[2] = (encode(value.charAt(iBuffer[2])) << 6) | encode(value.charAt(iBuffer[3]));
		chunk[3] = '\0';
		*result += chunk;
		delete[] chunk;
	}
	return result;
}

bool Utilities::compareFixedSizeArray(const String& value, const char fixedSizeArray[], size_t arraySize) {
	if (value.length() <= arraySize) {
		for (int i = 0; i < arraySize; i++) {
			if (value.charAt(i) != fixedSizeArray[i])
				return false;
		}
	} else return false;
	return true;
}
