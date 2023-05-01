/* c_introspect.h
 * Classes for introspecting into binary data blobs
 * using cStruct and friends (see table.h).
 * Allows you to get an enum telling you the type of 
 * each struct member, the name of the member, 
 * and a void pointer to it. Get an introspect::struct 
 * object by initializing it with a cStruct*.
 * You can read information about the struct out
 * after that. 
 *
 * Last, you can call `to_json()` on a Struct to 
 * get a workable JSON representation of the Struct.
 *************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stddef.h>
#include <string>
#include <cstring>
#include <type_traits>
#include <vector>
#include <map>
#include <any>

#include <iostream>


extern "C" {
#include "table.h"
}


namespace introspect {

	class abstractToJson {
		/* Interface for things that can be turned into json */
	public:
		virtual std::string to_json() = 0;
	};

	class abstractStructMember: public abstractToJson {
		/* Interface for members of Structs */
	public:
		virtual std::string& getName() = 0;
		virtual size_t getNumElements() = 0;
		virtual cType getTypeAsEnum() = 0;
		virtual void* getInnerPointer() = 0;
		virtual ~abstractStructMember() {};
	};

	template <typename T>
	class structMember: public abstractStructMember{
		/* Struct: An introspectable representation of a C struct */
		std::string name;
		T* elements;
		size_t count;
		cType typeEnum;
	public:
		using type = T;
		structMember(const cMember member):
			abstractStructMember(),
			name(member.name),
			elements(),
			count(member.elements),
			typeEnum(member.type){
			this->count = member.elements;
			this->elements = new T[this->count];
			std::memcpy(elements, member.value, sizeof(T)*this->count);
		}
		
		std::string to_json();

		~structMember() {
			delete[] elements;
			}

		std::string& getName() {return this->name;}
		size_t getNumElements() {return this->count;}
		cType getTypeAsEnum() {return this->typeEnum;}
		void* getInnerPointer() {return (void*)this->elements;}
	};
	
	class Struct: public abstractToJson{
		std::string name;
		std::vector<abstractStructMember*> elements;
		int mavType;
	public:
		int getNumChildren() {return elements.size();}
		int getType() {return mavType;}
		std::string& getName() {return this->name;}
		auto begin() {return this->elements.begin();}
		auto end() {return this->elements.end();}
		auto innerVector() {return elements;}
		Struct(const cStruct * s):
			name(s->name),
			elements(),
			mavType(s->mavType){
			for (int i = 0; i < s->numMembers; i++) {
				cMember cur = (s->members[i]);
				abstractStructMember * ref = nullptr;
				// I hate this.

				switch (cur.type) {

				case BOOL:
					ref = new structMember<bool>(cur);
					elements.push_back(ref);
					break;
				case CHAR:
					ref = new structMember<char>(cur);
					elements.push_back(ref);
					break;
				case SIGNED_CHAR:
					ref = new structMember<signed char>(cur);
					elements.push_back(ref);
					break;
				case UNSIGNED_CHAR:
					ref = new structMember<unsigned char>(cur);
					elements.push_back(ref);
					break;
				case SHORT:
					ref = new structMember<short>(cur);
					elements.push_back(ref);
					break;
				case UNSIGNED_SHORT:
					ref = new structMember<unsigned short>(cur);
					elements.push_back(ref);
					break;
				case INT:
					ref = new structMember<int>(cur);
					elements.push_back(ref);
					break;

				case UNSIGNED_INT:
					ref = new structMember<unsigned int>(cur);
					elements.push_back(ref);
					break;
				case LONG:
					ref = new structMember<long>(cur);
					elements.push_back(ref);
					break;
				case UNSIGNED_LONG:
					ref = new structMember<unsigned long>(cur);
					elements.push_back(ref);
					break;
				case LONG_LONG:
					ref = new structMember<long long>(cur);
					elements.push_back(ref);
					break;
				case UNSIGNED_LONG_LONG:
					ref = new structMember<unsigned long long>(cur);
					elements.push_back(ref);
					break;
				case INT8_T:
					ref = new structMember<int8_t>(cur);
					elements.push_back(ref);
					break;

				case UINT8_T:
					ref = new structMember<uint8_t>(cur);
					elements.push_back(ref);
					break;
				case INT16_T:
					ref = new structMember<int16_t>(cur);
					elements.push_back(ref);
					break;
				case UINT16_T:
					ref = new structMember<uint16_t>(cur);
					elements.push_back(ref);
					break;
				case INT32_T:
					ref = new structMember<int32_t>(cur);
					elements.push_back(ref);
					break;
				case UINT32_T:
					ref = new structMember<uint32_t>(cur);
					elements.push_back(ref);
					break;
				case INT64_T:
					ref = new structMember<int64_t>(cur);
					elements.push_back(ref);
					break;

				case UINT64_T:
					ref = new structMember<uint64_t>(cur);
					elements.push_back(ref);
					break;
				case FLOAT:
					ref = new structMember<float>(cur);
					elements.push_back(ref);
					break;
				case DOUBLE:
					ref = new structMember<double>(cur);
					elements.push_back(ref);
					break;
				case NONE:
					throw "tried to instantiate empty template!";
					break;
				}
			}
		}

		~Struct() {
			for (auto i = this->elements.begin(); i != this->elements.end(); i++) {
				delete (*i);
			}
			this->elements.clear();
		}

		std::string to_json();
		
	};

	template <typename T>
	std::string structMember<T>::to_json(){
		std::stringstream stream;
		stream << "    \"" << this->name << "\": ";
		if (this->count == 1){
			stream << std::to_string(*this->elements);
		}
		else if (this->count > 1 && this->typeEnum == CHAR ) {
			stream << "\""  << std::to_string(*this->elements) << "\"";
		}
		else {
		    stream <<  "[";
			for (size_t i = 0; i < this->count; i++) {
				if (i != 0) stream << ", ";
				stream << std::to_string(this->elements[i]);
			}
		    stream << "]";
		}
		return stream.str();
	}	
}

