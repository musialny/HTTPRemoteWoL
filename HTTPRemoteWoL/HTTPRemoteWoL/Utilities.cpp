/*
 * Utilities.cpp
 *
 * Created: 8/23/2021 10:52:58 PM
 *  Author: musialny
 */ 

#include "Utilities.h"
#include <Arduino.h>

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
			if (i == (*splitPoints)[index]) {
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