#pragma once
#include "par.h"
#include "thrift.h"
#include <stdlib.h>
#include <stdint.h>
#include <flecs.h>

char const * par_get_id_string(uint32_t id)
{
	switch(id)
	{
	case PAR_ID_TYPE: return "PAR_ID_TYPE";
	case PAR_ID_NAME: return "PAR_ID_NAME";
	default: return NULL;
	}
}


char const * par_get_type_string(uint32_t t)
{
	switch(t)
	{
	case PAR_BOOLEAN: return "PAR_BOOLEAN";
	case PAR_INT32: return "PAR_INT32";
	case PAR_INT64: return "PAR_INT64";
	case PAR_INT96: return "PAR_INT96";
	case PAR_FLOAT: return "PAR_FLOAT";
	case PAR_DOUBLE: return "PAR_DOUBLE";
	case PAR_BYTE_ARRAY: return "PAR_BYTE_ARRAY";
	case PAR_FIXED_LEN_BYTE_ARRAY: return "PAR_FIXED_LEN_BYTE_ARRAY";
	default: return "";
	}
}

void par_print_field(int32_t id, int32_t type, union thrift_value value)
{
	char buf[100] = {0};
	thrift_get_field_str(type, value, buf);
	char * idstr =  par_get_id_string(id);
	if(idstr)
	{
		printf("%15s = %20s : %s\n", idstr, buf, thrift_get_type_string(type));
	}
	else
	{
		printf("%15i = %20s : %s\n", id, buf, thrift_get_type_string(type));
	}
}



void push(struct thrift_context * ctx, int32_t id, int32_t type, union thrift_value value)
{
	par_print_field(id, type, value);
}

void par_read()
{
	FILE * file = NULL;
	ecs_os_fopen(&file, "../demo1/userdata1.parquet", "rb");
	int32_t l = 0;
	struct thrift_context ctx = {0};
	ctx.data = thrift_read_footer(file, &l);
	ctx.current = ctx.data;
	ctx.push = push;

	thrift_recursive_read(&ctx, 0, THRIFT_STRUCT);

	for(int32_t i = 0; i < l; ++i)
	{
		//printf("%2c : %02X\n", ctx.data[i], ctx.data[i]);
	}


	if(file) {fclose(file);}
	if(ctx.data) {ecs_os_free(ctx.data);}
}
