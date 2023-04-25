#include <iostream>
#include <string>
#include <sstream>

#include "c_introspect.h"


using namespace std;

extern "C" {
#include "mavlink/mavlink_types.h"
#include "mavlink/common/mavlink.h"
#include "table.h"
#include "mavlink_introspect_gen.h"
}


int main() {
	mavlink_status_t status;
	mavlink_message_t msg;
	int chan = MAVLINK_COMM_1;
	char byte = 0;
	int numBytes = 0;
	while (not cin.eof()) {
		byte = cin.get();
		if (mavlink_parse_char(chan, byte, &msg, &status)) {
			int id = msg.msgid;
			if (unboxers[id] != 0) {
				cerr << "Msg " << id << " received" << endl;
				cStruct * cRecord = (unboxers[msg.msgid])(&msg);
				introspect::Struct record = introspect::Struct(cRecord);
				cout << record.to_json();

			} else {
				cerr << "Nonexistant msg id " << id << " received; ignoring\n";
			}
		}
		numBytes++;
	}
	cerr << endl << numBytes << " bytes" << endl;
	return 0;
}
