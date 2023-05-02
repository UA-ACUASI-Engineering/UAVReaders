#include <ostream>
#include <string>
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
			long index = (long)description.members[i].value;
			uint8_t * mem = copy.rest;
			current.members[i].value = (void*)(mem + index);
			/*				std::cerr << index
							<< std::endl
							<< &copy.rest
							<< std::endl
							<< current.members[i].value
							<< std::endl<<std::endl;
			*/
			std::cerr.flush();
		}
			
		return current;
	}
	void DFFormatDescription::releaseFormat() {
		delete[] current.members;
	}

	bool DFFormatDescription::initialized = false;
	cMember DFFormatDescription::dfMemberTypes[256] = {cMember{0,0,0,NONE}};
	void DFFormatDescription::initialize() {
		if (not DFFormatDescription::initialized){
			DFFormatDescription::initialized = true;
			DFFormatDescription::dfMemberTypes['B'] = cMember{0, 0, 1, UINT8_T};
			DFFormatDescription::dfMemberTypes['C'] = cMember{0, 0, 100, UINT16_T};
			DFFormatDescription::dfMemberTypes['E'] = cMember{0, 0, 100, UINT32_T};
			DFFormatDescription::dfMemberTypes['H'] = cMember{0, 0, 1, UINT16_T};
			DFFormatDescription::dfMemberTypes['I'] = cMember{0, 0, 1, UINT32_T};
			DFFormatDescription::dfMemberTypes['L'] = cMember{0, 0, 1, INT32_T};
			DFFormatDescription::dfMemberTypes['M'] = cMember{0, 0, 1, UINT8_T};
			DFFormatDescription::dfMemberTypes['N'] = cMember{0, 0, 16, CHAR};
			DFFormatDescription::dfMemberTypes['Q'] = cMember{0, 0, 1, UINT64_T};
			DFFormatDescription::dfMemberTypes['Z'] = cMember{0, 0, 64, CHAR};
			DFFormatDescription::dfMemberTypes['a'] = cMember{0, 0, 32, INT16_T};
			DFFormatDescription::dfMemberTypes['b'] = cMember{0, 0, 1, INT8_T};
			DFFormatDescription::dfMemberTypes['c'] = cMember{0, 0, 100, INT16_T};
			DFFormatDescription::dfMemberTypes['d'] = cMember{0, 0, 1, DOUBLE};
			DFFormatDescription::dfMemberTypes['e'] = cMember{0, 0, 100, INT32_T};
			DFFormatDescription::dfMemberTypes['f'] = cMember{0, 0, 1, FLOAT};
			DFFormatDescription::dfMemberTypes['h'] = cMember{0, 0, 1, INT16_T};
			DFFormatDescription::dfMemberTypes['i'] = cMember{0, 0, 1, INT32_T};
			DFFormatDescription::dfMemberTypes['n'] = cMember{0, 0, 4, CHAR};
			DFFormatDescription::dfMemberTypes['q'] = cMember{0, 0, 1, INT64_T};
		}
	}

	const uint8_t DFParser::headerByte1 = 0xA3;
	const uint8_t DFParser::headerByte2 = 0x95;
	const uint8_t DFParser::idDescriptorPacketByte = 0x80;
	const uint8_t DFParser::idDescriptorPacketLen = 0x59;

	void DFParser::newFormat(const DFDescriptionPacket& packet) {
		uint8_t type = packet.type;
		if (this->formats[type] != nullptr) 
			delete formats[type];
		this->formats[type] = new DFFormatDescription(packet);
		this->packetLengths[packet.type] = packet.len;
	}
	
	bool DFParser::parseDataFlash(const uint8_t byte, DFPacket& packet) {
		if (this->consumedCount == 0 and byte == DFParser::headerByte1) {
			this->consumedCount = 1;
			goto out;
		}

		else if (this->consumedCount == 1 and byte == DFParser::headerByte2){
			this->consumedCount = 2;
			goto out;
		}

		else if (this->consumedCount == 2 and byte == DFParser::idDescriptorPacketByte) {
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

		else if (this->consumedCount >= 3 and this->consumedCount < (this->totalSize-1)) {
			// Bytes of the packet body
			packet.rest[this->consumedCount - 3] = byte;
			this->consumedCount++;
			goto out;
		}

		else if (this->consumedCount >= 3 and this->consumedCount == (this->totalSize-1)) {
			/// Last byte in the packet
			if (not this->isDescriptor) {
				packet.rest[this->consumedCount - 3] = byte;
				this->zero();
				return true;
			}

			// The parser does not emit format packets, just keeps them for itself.
			else { 
				packet.rest[this->consumedCount - 3] = byte;
				const DFDescriptionPacket format = reinterpret_cast<DFDescriptionPacket&>(packet);
				this->newFormat(format);
				/*				std::cerr << "defined packet 0x"
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
}
