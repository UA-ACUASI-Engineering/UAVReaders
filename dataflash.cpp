#include <ostream>
#include <string>
#include <cstdint>
#include "dataflash.h"
#include "table.h"

#include <iostream>
#include <iomanip>

namespace DataFlash{
	
	const cStruct& DFFormatDescription::format(DFPacket& packet) {
		/* Deep copy `description` and set the pointers to point into my packet copy*/
		if (packet.packet_type != this->packet_type) {
			
		}
		if (current.members != nullptr) {
			delete[] current.members;
		}
		this->copy = packet;
		this->current = description;
		this->current.members = new cMember[description.numMembers];
		for (int i = 0; i < current.numMembers; i++){
			current.members[i] = description.members[i];
			//bleh
			uint64_t index = (uintptr_t)description.members[i].value;
			uint8_t * mem = copy.rest;
			current.members[i].value = (void*)(mem + index);
			/*				std::cerr << index
							<< std::endl
							<< &copy.rest
							<< std::endl
							<< current.members[i].value
							<< std::endl<<std::endl;
			//*/
			std::cerr.flush();
		}
			
		return current;
	}
	void DFFormatDescription::releaseFormat() {
		delete[] current.members;
	}

	bool DFFormatDescription::initialized = false;
	cMember DFFormatDescription::dfMemberTypes[256] = {{.name=0,.value=0,.elements=0,.type=NONE}};
	void DFFormatDescription::initialize() {
		if (! DFFormatDescription::initialized){
			DFFormatDescription::initialized = true;
			cMember elem_B = {.name = 0,.value=0,.elements=1,.type = UINT8_T};
			cMember elem_C = {.name=0,.value= 0,.elements= 1,.type= UINT16_T};
			cMember elem_E = {.name=0,.value= 0,.elements= 1,.type= UINT32_T};
			cMember elem_H = {.name=0,.value= 0,.elements= 1,.type= UINT16_T};
			cMember elem_I = {.name=0,.value= 0,.elements= 1,.type= UINT32_T};
			cMember elem_L = {.name=0,.value= 0,.elements= 1,.type= INT32_T};
			cMember elem_M = {.name=0,.value= 0,.elements= 1,.type= UINT8_T};
			cMember elem_N = {.name=0,.value= 0,.elements= 16,.type= CHAR};
			cMember elem_Q = {.name=0,.value= 0,.elements= 1,.type= UINT64_T};
			cMember elem_Z = {.name=0,.value= 0,.elements= 64,.type= CHAR};
			cMember elem_a = {.name=0,.value= 0,.elements= 32,.type= INT16_T};
			cMember elem_b = {.name=0,.value= 0,.elements= 1,.type= INT8_T};
			cMember elem_c = {.name=0,.value= 0,.elements= 1,.type= INT16_T};
			cMember elem_d = {.name=0,.value= 0,.elements= 1,.type= DOUBLE};
			cMember elem_e = {.name=0,.value= 0,.elements= 1,.type= INT32_T};
			cMember elem_f = {.name=0,.value= 0,.elements= 1,.type= FLOAT};
			cMember elem_h = {.name=0,.value= 0,.elements= 1,.type= INT16_T};
			cMember elem_i = {.name=0,.value= 0,.elements= 1,.type= INT32_T};
			cMember elem_n = {.name=0,.value= 0,.elements= 4,.type= CHAR};
			cMember elem_q = {.name=0,.value= 0,.elements= 1,.type= INT64_T};
			DFFormatDescription::dfMemberTypes['B'] = elem_B;
			DFFormatDescription::dfMemberTypes['C'] = elem_C;			
			DFFormatDescription::dfMemberTypes['E'] = elem_E;
			DFFormatDescription::dfMemberTypes['H'] = elem_H;
			DFFormatDescription::dfMemberTypes['I'] = elem_I;
			DFFormatDescription::dfMemberTypes['L'] = elem_L;
			DFFormatDescription::dfMemberTypes['M'] = elem_M;
			DFFormatDescription::dfMemberTypes['N'] = elem_N;
			DFFormatDescription::dfMemberTypes['Q'] = elem_Q;
			DFFormatDescription::dfMemberTypes['Z'] = elem_Z;
			DFFormatDescription::dfMemberTypes['a'] = elem_a;
			DFFormatDescription::dfMemberTypes['b'] = elem_b;
			DFFormatDescription::dfMemberTypes['c'] = elem_c;
			DFFormatDescription::dfMemberTypes['d'] = elem_d;
			DFFormatDescription::dfMemberTypes['e'] = elem_e;
			DFFormatDescription::dfMemberTypes['f'] = elem_f;
			DFFormatDescription::dfMemberTypes['h'] = elem_h;
			DFFormatDescription::dfMemberTypes['i'] = elem_i;
			DFFormatDescription::dfMemberTypes['n'] = elem_n;
			DFFormatDescription::dfMemberTypes['q'] = elem_q;
		}
	}

	const uint8_t DFParser::headerByte1 = 0xA3;
	const uint8_t DFParser::headerByte2 = 0x95;
	const uint8_t DFParser::idDescriptorPacketByte = 0x80;
	const uint8_t DFParser::idDescriptorPacketLen = 0x59;
	char DFFormatDescription::defaultName[10] = "no name";
 
	void DFParser::newFormat(const DFDescriptionPacket& packet) {
		uint8_t type = packet.type;
		if (! ( packet.name && packet.fmt_string && packet.labels))
			return;
		if (this->formats[type] != nullptr) 
			delete formats[type];
		this->formats[type] = new DFFormatDescription(packet);
		this->packetLengths[packet.type] = packet.len;
		return;
	}
	
	bool DFParser::parseDataFlash(const uint8_t byte, DFPacket& packet) {
		if (this->consumedCount == 0 && byte == DFParser::headerByte1) {
			this->consumedCount = 1;
			goto out;
		}

		else if (this->consumedCount == 1 && byte == DFParser::headerByte2){
			this->consumedCount = 2;
			goto out;
		}

		else if (this->consumedCount == 2 && byte == DFParser::idDescriptorPacketByte) {
			// the packet is a format descriptor
			this->totalSize = DFParser::idDescriptorPacketLen;
			this->consumedCount = 3;
			this->isDescriptor = true;
			packet.packet_type = DFParser::idDescriptorPacketByte;
			goto out;
		}

		else if (this->consumedCount == 2) {
			// some other packet type
			this->consumedCount = 3;
			packet.packet_type = byte;
			this->totalSize = this->packetLengths[byte];
			goto out;
		}

		else if (this->consumedCount >= 3 && this->consumedCount < (this->totalSize-1)) {
			// Bytes of the packet body
			packet.rest[this->consumedCount - 3] = byte;
			this->consumedCount++;
			goto out;
		}

		else if (this->consumedCount >= 3 && this->consumedCount == (this->totalSize-1)) {
			/// Last byte in the packet
			if (! this->isDescriptor) {
				packet.rest[this->consumedCount - 3] = byte;
				this->zero();
				return true;
			}

			// The parser does not emit format packets, just keeps them for itself.
			else {
				packet.rest[this->consumedCount - 3] = byte;
				const DFDescriptionPacket format = reinterpret_cast<DFDescriptionPacket&>(packet);
				this->newFormat(format);
				/*std::cerr << "defined packet 0x"
						  << std::setfill('0')
						  << std::setw(2)
						  << std::hex
						  <<(int)this->formats[format.type]->packet_type
						  << std::endl;*/
				
			}
		}
		
		this->zero();
	out:
		return false;
	}

	const cStruct& DFParser::getIntrospectiveStruct(DFPacket& packet) {
		uint8_t type = packet.packet_type;
		if (this->formats[type] != nullptr) {
			return this->formats[type]->format(packet);
		}
		else throw "unknown packet type";
	}

	bool DFParser::formatExists(uint8_t byte) {
		return this->formats[byte] != nullptr;
	}
	std::string DFParser::formatName(uint8_t byte){
		return this->formats[byte]->getName();
	}
}
