/* 
* ElasticArray.h
*
* Created: 8/24/2021 1:21:47 AM
* Author: musialny
*/

#ifndef __ELASTICARRAY_H__
#define __ELASTICARRAY_H__

template<typename T>
class ElasticArray
{
public:
	T* array;
	int arrayLength;
	int stackPointer;
public:
	ElasticArray() {
		this->array = new T;
		this->arrayLength = 1;
		this->stackPointer = 0;
	}
	~ElasticArray() {
		delete[] this->array;
	}
	
	ElasticArray& push(T value) {
		if (this->arrayLength - 1 < this->stackPointer) {
			int arrayLength = this->arrayLength + 1; //Smallest allocation for better memory usage
			// int arrayLength = this->arrayLength * 2;
			T* newArray = new T[arrayLength];
			for (int i = 0; i < this->arrayLength; i++)
				newArray[i] = this->array[i];
			delete[] this->array;
			this->array = newArray;
			this->arrayLength = arrayLength;
			this->array[this->stackPointer++] = value;
		} else this->array[this->stackPointer++] = value;
		return *this;
	}
	
	const T& operator[](int index) const {
		return this->array[index];
	}
	
	int length() const {
		return this->stackPointer;
	}
};

#endif //__ELASTICARRAY_H__
