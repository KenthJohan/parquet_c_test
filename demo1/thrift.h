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

/*
  BOOLEAN = 0;
  INT32 = 1;
  INT64 = 2;
  INT96 = 3;
  FLOAT = 4;
  DOUBLE = 5;
  BYTE_ARRAY = 6;
  FIXED_LEN_BYTE_ARRAY = 7;
  */

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
	uint8_t * data;
	uint8_t * current;
	int32_t length;
	int32_t last_field_id;
	int (*push)(struct thrift_context * ctx, int32_t id, int32_t type, union thrift_value value);
};


uint8_t * thrift_read_footer(FILE * file, int32_t * out_length);
void thrift_recursive_read(struct thrift_context * ctx, int32_t id, int32_t type);
char * thrift_get_type_string(uint32_t t);
void thrift_get_field_str(int32_t type, union thrift_value value, char * buf);
