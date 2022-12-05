#pragma once
#include "parquet.h"
#include "thrift.h"
#include <stdlib.h>
#include <stdint.h>
#include <flecs.h>

char const * parquet_get_id_string(uint32_t id)
{
	switch(id)
	{
    case PARQUET_ID_TYPE: return "PARQUET_ID_TYPE";
    case PARQUET_ID_NAME: return "PARQUET_ID_NAME";
	default: return NULL;
	}
}


char const * parquet_get_type_string(uint32_t t)
{
	switch(t)
	{
    case PARQUET_BOOLEAN: return "PARQUET_BOOLEAN";
    case PARQUET_INT32: return "PARQUET_INT32";
    case PARQUET_INT64: return "PARQUET_INT64";
    case PARQUET_INT96: return "PARQUET_INT96";
    case PARQUET_FLOAT: return "PARQUET_FLOAT";
    case PARQUET_DOUBLE: return "PARQUET_DOUBLE";
    case PARQUET_BYTE_ARRAY: return "PARQUET_BYTE_ARRAY";
    case PARQUET_FIXED_LEN_BYTE_ARRAY: return "PARQUET_FIXED_LEN_BYTE_ARRAY";
	default: return "";
	}
}

void parquet_print_field(int32_t id, int32_t type, union thrift_value value)
{
	char buf[100] = {0};
	thrift_get_field_str(type, value, buf);
    char const * idstr =  parquet_get_id_string(id);
	if(idstr)
	{
		printf("%15s = %20s : %s\n", idstr, buf, thrift_get_type_string(type));
	}
	else
	{
		printf("%15i = %20s : %s\n", id, buf, thrift_get_type_string(type));
	}
}


void parquet_read_footer(struct thrift_context * ctx, FILE * file)
{
    char par1[4] = {0};
    char par2[4] = {0};
    int32_t l = 0;
    fseek(file, 0, SEEK_SET);
    fread(par1, sizeof(par1), 1, file);
    fseek(file, -8, SEEK_END);
    fread(&l, sizeof(int32_t), 1, file);
    fseek(file, 0, SEEK_SET);
    fread(par2, sizeof(par2), 1, file);
    fseek(file, -8-l, SEEK_END);
    ctx->data = ecs_os_malloc(l);
    ctx->current = ctx->data;
    fread(ctx->data, l, 1, file);
    ctx->length = l;
}


void push(struct thrift_context * ctx, int32_t id, int32_t type, union thrift_value value)
{
    parquet_print_field(id, type, value);
}

void parquet_testcase1()
{
	FILE * file = NULL;
	ecs_os_fopen(&file, "../demo1/userdata1.parquet", "rb");
	int32_t l = 0;
	struct thrift_context ctx = {0};
    ctx.push = push;
    parquet_read_footer(&ctx, file);
	thrift_recursive_read(&ctx, 0, THRIFT_STRUCT);

	for(int32_t i = 0; i < l; ++i)
	{
		//printf("%2c : %02X\n", ctx.data[i], ctx.data[i]);
	}


	if(file) {fclose(file);}
	if(ctx.data) {ecs_os_free(ctx.data);}
}






