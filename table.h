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
	NONE = 23
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
	uint16_t mavType;
} cStruct;

const int cTypeSize[] = {
    0,     // Not defined
    1,     // BOOL
    1,     // CHAR
    1,     // SIGNED_CHAR
    1,     // UNSIGNED_CHAR
    2,     // SHORT
    2,     // UNSIGNED_SHORT
    4,     // INT
    4,     // UNSIGNED_INT
    4,     // LONG
    4,     // UNSIGNED_LONG
    8,     // LONG_LONG
    8,     // UNSIGNED_LONG_LONG
    1,     // INT8_T
    1,     // UINT8_T
    2,     // INT16_T
    2,     // UINT16_T
    4,     // INT32_T
    4,     // UINT32_T
    8,     // INT64_T
    8,     // UINT64_T
    4,     // FLOAT
    8,     // DOUBLE
	0,     // NONE
};
