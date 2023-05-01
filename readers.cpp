#include <cstdint>
#include "readers.h"
#include "table.h"

extern "C"{
#include "table.h"
#include "mavlink/mavlink_types.h"
#include "mavlink/common/mavlink.h"
#include "mavlink_introspect_gen.h"
}

#include "c_introspect.h"

namespace UAVFormatReaders {
	void dataFlashReader::parseByte(const uint8_t byte) {
		this->hasData = this->parser.parseDataFlash(byte, this->msg);
		this->numBytes++;
		if (this->hasData) {
			this->numPackets++;
			cStruct s = this->parser.getIntrospectiveStruct(this->msg);
			this->results.push(new introspect::Struct(&s));
		}
	}

	void dataFlashReader::parseBuffer(const uint8_t * buf, uint64_t len) {
		for (uint64_t i = 0; i < len; i++) {
		    this->parseByte(buf[i]);
		}
	}

	introspect::Struct* dataFlashReader::getPacket() {
		// Caller should delete
		if (this->numAvailable() > 0) {
			auto s = this->results.front();
			this->results.pop();
			return s;
		}
		else throw "Not ready to provide data!";
	}


	int mavLinkReader::channel = 1;
	void mavLinkReader::parseByte(const uint8_t byte){
		this->hasData = mavlink_parse_char(
										   this->mychannel,
										   byte,
										   &this->msg,
										   &this->status);
		this->numBytes++;
		if (this->hasData){
			this->numPackets++;
			int id = this->msg.msgid;
			if (unboxers[id] != nullptr) {
				cStruct * s = (unboxers[id])(&this->msg);
				this->results.push(new introspect::Struct(s));
			}
		}
	}

	void mavLinkReader::parseBuffer(const uint8_t * buf, uint64_t len){
		for (uint64_t i = 0; i < len; i++) {
			this->parseByte(buf[i]);
		}
	}
	introspect::Struct * mavLinkReader::getPacket() {
		// Caller should delete
		if (this->numAvailable() > 0) {
			auto s = this->results.front();
			this->results.pop();
			return s;
		}
		else throw "Not ready to provide data!";
	}

	void deleteStruct(introspect::Struct * s) {
		delete s;
	}
}
