#pragma once
#include <stdint.h>
#include <stdio.h>
#include "thrift.h"


#define PARQUET_BOOLEAN 0
#define PARQUET_INT32 1
#define PARQUET_INT64 2
#define PARQUET_INT96 3
#define PARQUET_FLOAT 4
#define PARQUET_DOUBLE 5
#define PARQUET_BYTE_ARRAY 6
#define PARQUET_FIXED_LEN_BYTE_ARRAY 7

#define PARQUET_ID_TYPE 1
#define PARQUET_ID_NAME 4


void parquet_read_footer(struct thrift_context * ctx, FILE * file);
void parquet_testcase1();


