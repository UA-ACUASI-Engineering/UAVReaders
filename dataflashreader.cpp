#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include "dataflash.h"
#include "c_introspect.h"

extern "C" {
#include "table.h"
}

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::setfill;
using std::setw;

int main() {
	DataFlash::DFParser parser;
	DataFlash::DFPacket msg;
	char byte = 0;
	int numBytes = 0;
	int numPackets = 0;
	cout << "[\n";
	bool first = true;
	while (! cin.eof()) {
		byte = cin.get();
		if (parser.parseDataFlash(byte, msg)) {
			if (! first) cout << ",\n";
			first = false;
			numPackets++;
			uint8_t id = msg.packet_type;
			if (parser.formatExists(id)){
				cerr << "Msg 0x"
					 << setfill('0')
					 << setw(2)
					 << std::hex
					 << (int)id
					 << " received"
					 << endl;
				cStruct cRecord = parser.getIntrospectiveStruct(msg);
				introspect::Struct record = introspect::Struct(&cRecord);
				
				cout << record.to_json();
			}

			else {
				cerr << "Nonexistant msg id " << id << " received; ignoring\n";
			}
		}
		numBytes++;
	}
	cout << "]";
	cerr << endl << numBytes << " bytes" << endl;
	cerr << numPackets << " packets" << endl;
	return 0;
}
