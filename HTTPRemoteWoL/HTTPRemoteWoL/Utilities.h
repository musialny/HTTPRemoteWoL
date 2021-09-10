/*
 * Utilities.h
 *
 * Created: 8/23/2021 10:53:07 PM
 *  Author: musialny
 */ 

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include "WString.h"
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
	SplittedString* split(const String& value, int maxArraySize);
}

#endif /* UTILITIES_H_ */
