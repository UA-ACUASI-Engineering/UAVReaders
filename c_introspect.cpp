#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
#include <any>
#include <format>

#include "table"

namespae introspect {
	class abstractStructMember {
	};

	template <typename T>
	class structMember<T>: abstractStructMember {
		std::string name;
		T * element;
		size_t count;
	};
	
	class CStruct{
		std::string name;
		std::vector<abstractStructMember> elements;
	};
	
}
