/* readers.h
 * Classes for easily reading things from MavLink and 
 * DataFlash binary files
 * 
 * Contains UAVformatreaders::abstractReader ,
 * UAVFormatreaders::dataFlashReader ,
 * and UAVformatreaders::mavlinkreader
 *************************************************/

#include <cstdint>
#include <queue>

#include "dataflash.h"
#include "c_introspect.h"

extern "C"{
#include "table.h"
#include "mavlink/mavlink_types.h"
#include "mavlink/common/mavlink.h"
}


namespace UAVFormatReaders {

	class abstractReader {
		/* Interface for packet Readers */
		virtual void parseByte(const uint8_t) = 0;
		virtual void parseBuffer(const uint8_t *, uint64_t) = 0;
		virtual int numAvailable() = 0;
		virtual int bytesSeen() = 0;
		virtual int  packetsSeen() = 0;
		virtual introspect::Struct* getPacket() = 0;
	};
	
	class dataFlashReader: public abstractReader {
		DataFlash::DFParser parser;
		DataFlash::DFPacket msg;
		std::queue<introspect::Struct *> results;
		bool hasData;
		uint64_t numBytes;
		uint64_t numPackets;
	public:
		dataFlashReader():
			parser(),
			msg(),
			results(),
			hasData(false),
			numBytes(0),
			numPackets(0) {
		}
		void parseByte(const uint8_t);
		void parseBuffer(const uint8_t *, uint64_t len);
		int numAvailable() {return this->results.size();}
		int bytesSeen() {return this->numBytes;}
		int packetsSeen() {return this->numPackets;}
		introspect::Struct* getPacket();
		
	};


	class mavLinkReader: public abstractReader {
		static int channel;
		int mychannel;
		mavlink_status_t status;
		mavlink_message_t msg;
		std::queue<introspect::Struct *> results;
		bool hasData;
		uint64_t numBytes;
		uint64_t numPackets;
	public:
		mavLinkReader():
			mychannel(mavLinkReader::channel),
			status(),
			msg(),
			results(),
			hasData(false),
			numBytes(0),
			numPackets(0) {
			mavLinkReader::channel++;
		}
		void parseByte(const uint8_t);
		void parseBuffer(const uint8_t * buf, uint64_t len);
		int numAvailable() {return this->results.size();}
		int bytesSeen() {return this->numBytes;}
		int packetsSeen() {return this->numPackets;}
		introspect::Struct* getPacket();
	};

	void deleteStruct(introspect::Struct *);
}
