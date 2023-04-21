/* Table.h
 * Defines osome types used in the unboxer table
 * generator and associated mecanics.
 *************************************************/
#pragma once

#include <stdint.h>


typedef enum{
    BOOL = 1,
    CHAR = 2,
    SIGNED_CHAR = 3,
    UNSIGNED_CHAR = 4,
    SHORT = 5,
    UNSIGNED_SHORT = 6,
    INT = 7,
    UNSIGNED_INT = 8,
    LONG = 9,
    UNSIGNED_LONG = 10,
    LONG_LONG = 11,
    UNSIGNED_LONG_LONG = 12,
    INT8_T = 13,
    UINT8_T = 14,
    INT16_T = 15,
    UINT16_T = 16,
    INT32_T = 17,
    UINT32_T = 18,
    INT64_T = 19,
    UINT64_T = 20,
    FLOAT = 21,
    DOUBLE = 22,
} cType;


typedef struct {
	char * name;
    void * value;
	int elements;
	cType type;
} cMember;

typedef struct {
	char * name;
	cMember * members;
	uint16_t numMembers;
} cStruct;
