#pragma once
#include <stdint.h>
#include <stdio.h>

#define THRIFT_STOP 0x00
#define THRIFT_BOOLEAN_TRUE 0x01
#define THRIFT_BOOLEAN_FALSE 0x02
#define THRIFT_BYTE 0x03
#define THRIFT_I16 0x04
#define THRIFT_I32 0x05
#define THRIFT_I64 0x06
#define THRIFT_DOUBLE 0x07
#define THRIFT_BINARY 0x08
#define THRIFT_LIST 0x09
#define THRIFT_SET 0x0A
#define THRIFT_MAP 0x0B
#define THRIFT_STRUCT 0x0C

union thrift_value
{
	uint64_t value_u64;
	int64_t value_i64;
	struct
	{
		int32_t list_type;
		int32_t list_size;
	};
	struct
	{
		int32_t string_size;
		char * string_data;
	};
};


struct thrift_context
{
	uint8_t * data_start;
	uint8_t * data_end;
	uint8_t * data_current;
	int32_t last_field_id;
    void (*cb_field)(struct thrift_context * ctx, int32_t id, int32_t type, union thrift_value value);
};



void thrift_recursive_read(struct thrift_context * ctx, int32_t id, int32_t type);
void thrift_get_field_str(int32_t type, union thrift_value value, char * buf);
char const * thrift_get_type_string(uint32_t t);
