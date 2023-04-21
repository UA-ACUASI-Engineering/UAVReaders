/* Table.h
 * Defines osome types used in the unboxer table
 * generator and associated mecanics.
 *************************************************/
#pragma once

#include <stdint.h>


typedef enum{
    BOOL = 1,
    CHAR,
    SIGNED_CHAR,
    UNSIGNED_CHAR,
    SHORT,
    UNSIGNED_SHORT,
    INT,
    UNSIGNED_INT,
    LONG,
    UNSIGNED_LONG,
    LONG_LONG,
    UNSIGNED_LONG_LONG,
    INT8_T,
    UINT8_T,
    INT16_T,
    UINT16_T,
    INT32_T,
    UINT32_T,
    INT64_T,
    UINT64_T,
    FLOAT,
    DOUBLE,
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
