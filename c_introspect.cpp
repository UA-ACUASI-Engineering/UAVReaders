#include <string>
#include<sstream>

#include "c_introspect.h"

namespace introspect {	

	std::string Struct::to_json() {
		std::stringstream obj;
		obj << "{\n";
		obj << "    \"packet_name\": \"" << this->name << "\",\n    \"packet_id\": " << this->mavType;
		for (auto i = this->elements.begin(); i != this->elements.end(); i++){
			obj << ",\n";
			obj << (*i)->to_json();
		}
		obj << "\n}\n";
		return obj.str();
	}
}
