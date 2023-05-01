/* dataflash.h
 * dataflash reader
 * This is a bit lower level than the reader classes
 * in readers.h. Use those instead
 * Implements a parser for the DataFlash UAV logging 
 * format. 
 *************************************************/

#pragma once

#include <cstdint>
#include <queue>
#include <cstring>

extern "C"{
#include "table.h"
}

#define LEN_NAME 4
#define LEN_FMT_STR 16
#define LEN_LABELS 64

namespace DataFlash{

	extern "C"{
		struct __attribute__((packed)) DFPacket {
			uint16_t begin;
			uint8_t packet_type;
			uint8_t rest[253];
		};

		struct __attribute__((packed)) DFDescriptionPacket{
			uint16_t begin;
			uint8_t packet_type;
			uint8_t type;
			uint8_t len;
			char name[LEN_NAME];
			char fmt_string[LEN_FMT_STR];
			char labels[LEN_LABELS];
		};
	}

	class DFFormatDescription {
		static bool initialized;
		static void initialize();
		static cMember dfMemberTypes[256];
		char name[LEN_NAME + 1];
		char fields[LEN_LABELS + 1];

		uint8_t length;
		cStruct description;
		cStruct current;
		DFPacket copy;

	public:
		uint8_t packet_type;
		DFFormatDescription(const DFDescriptionPacket &format):
			name(),
			fields(),
			length(format.len),
			description(),
			current(),
			copy(),
			packet_type(format.type) {
			DFFormatDescription::initialize();
			description.mavType = format.type;

			name[LEN_NAME] = 0;
			fields[LEN_LABELS] = 0;
			std::memcpy(name, format.name, LEN_NAME * sizeof(char));
			std::memcpy(fields, format.labels, LEN_LABELS * sizeof(char));

			std::queue<char*> fieldNames;
			fieldNames.push(fields);
			for (int i = 0; i < LEN_LABELS and fields[i] != 0; i++) {
				if (fields[i] == ',') {
					fields[i] = 0;
					fieldNames.push(fields + i + 1);
				}
			}
 
			description.numMembers = 0;
			description.name = name;
			/* intentionally empty */
			for (; description.numMembers < LEN_FMT_STR and
					 format.fmt_string[description.numMembers] != 0;
				 description.numMembers++);
			description.members = new cMember[description.numMembers];

			long offset = 0;
			for (int i = 0; i < description.numMembers; i++) {
				char format_type = format.fmt_string[i];
				description.members[i] =
					DFFormatDescription::dfMemberTypes[(uint8_t)format_type];
				description.members[i].name = fieldNames.front();
				description.members[i].value = (void*)offset;
				fieldNames.pop();
				offset += cTypeSize[description.members[i].type];
			}
			current.members = nullptr;
			
		}

		~DFFormatDescription() {
			delete[] description.members;
			if (current.members != nullptr){
				delete[] current.members;
			}
		}

		const cStruct& format(DFPacket& packet);
		void releaseFormat();
	};

	class DFParser {
		const static uint8_t headerByte1;
		const static uint8_t headerByte2;
		const static uint8_t idDescriptorPacketByte;
		const static uint8_t idDescriptorPacketLen;
		uint8_t packetLengths[256];
		int headerCnt;
		int consumedCount;
		int totalSize;
		bool isDescriptor;
		DFFormatDescription * formats[256];
		void zero() {
			this->headerCnt = 0;
			this->consumedCount = 0;
			this->totalSize = 0;
			this->isDescriptor = false;
		}

	public:
		DFParser():
			packetLengths(),
			headerCnt(0),
			consumedCount(0),
			totalSize(0),
			isDescriptor(false),
			formats(){
			for (int i = 0; i < 256; i++) {
				formats[i] = nullptr;
				packetLengths[i] = 0;
			}
		}

		~DFParser () {
			for (int i =0; i < 256; i++)
				if (formats[i] != nullptr)
					delete formats[i];
		}

		bool parseDataFlash(const uint8_t byte, DFPacket& packet);
		const cStruct& getIntrospectiveStruct(DFPacket& packet);
		void releaseCStuctFormatter();
		void newFormat(const DFDescriptionPacket& packet);
		bool formatExists(uint8_t byte);
	};

}
