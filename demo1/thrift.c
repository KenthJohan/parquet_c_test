#pragma once
#include "thrift.h"
#include <stdint.h>
#include <flecs.h>

// https://github.com/apache/thrift/blob/master/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc

int64_t thrift_zigzag_to_i32(uint64_t n)
{
	return (int) (n >> 1) ^ -(int) (n & 1);
}

int64_t thrift_read_varint_i64(struct thrift_context * ctx)
{
	int rsize = 0;
	int lo = 0;
	int hi = 0;
	int shift = 0;
	while (1)
	{
		uint8_t b = ctx->current[0];
		ctx->current++;
		rsize ++;
		if (shift <= 25)
		{
			lo = lo | ((b & 0x7f) << shift);
		}
		else if (25 < shift && shift < 32)
		{
			lo = lo | ((b & 0x7f) << shift);
			hi = hi | ((b & 0x7f) >> (32-shift));
		}
		else
		{
			hi = hi | ((b & 0x7f) << (shift-32));
		}
		shift += 7;
		if (!(b & 0x80)){break;}
		if (rsize >= 10)
		{
			printf("Variable-length int over 10 bytes.");
			exit(1);
		}
	}
	//return new Int64(hi, lo);
	return lo;
}

int64_t thrift_read_zigzag_i64(struct thrift_context * ctx)
{
	int64_t value = thrift_read_varint_i64(ctx);
	return thrift_zigzag_to_i32(value);
}












uint8_t * thrift_read_footer(FILE * file, int32_t * out_length)
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
	uint8_t * footer = ecs_os_malloc(l);
	fread(footer, l, 1, file);
	(*out_length) = l;
	return footer;
}


char const * thrift_get_type_string(uint32_t t)
{
	switch(t)
	{
	case THRIFT_STOP: return "THRIFT_STOP";
	case THRIFT_BOOLEAN_TRUE: return "THRIFT_BOOLEAN_TRUE";
	case THRIFT_BOOLEAN_FALSE: return "THRIFT_BOOLEAN_FALSE";
	case THRIFT_BYTE: return "THRIFT_BYTE";
	case THRIFT_I16: return "THRIFT_I16";
	case THRIFT_I32: return "THRIFT_I32";
	case THRIFT_I64: return "THRIFT_I64";
	case THRIFT_DOUBLE: return "THRIFT_DOUBLE";
	case THRIFT_BINARY: return "THRIFT_BINARY";
	case THRIFT_LIST: return "THRIFT_LIST";
	case THRIFT_SET: return "THRIFT_SET";
	case THRIFT_MAP: return "THRIFT_MAP";
	case THRIFT_STRUCT: return "THRIFT_STRUCT";
	default: return "";
	}
}

void thrift_get_field_str(int32_t type, union thrift_value value, char * buf)
{
	int n = 20;
	switch(type)
	{
	case THRIFT_STOP: break;
	case THRIFT_BOOLEAN_TRUE: snprintf(buf, n, "true"); break;
	case THRIFT_BOOLEAN_FALSE: snprintf(buf, n, "false"); break;
	case THRIFT_BYTE: snprintf(buf, n, "%02X", value.value_u64); break;
	case THRIFT_I16: snprintf(buf, n, "%jd", value.value_i64); break;
	case THRIFT_I32: snprintf(buf, n, "%jd", value.value_i64); break;
	case THRIFT_I64: snprintf(buf, n, "%jd", value.value_i64); break;
	case THRIFT_DOUBLE: snprintf(buf, n, "%f", value.value_i64); break;
	case THRIFT_BINARY: snprintf(buf, n, "%.*s", value.string_size, value.string_data); break;
	case THRIFT_LIST: snprintf(buf, n, "%i of %s", value.list_size, thrift_get_type_string(value.list_type)); break;
	case THRIFT_SET: snprintf(buf, n, ""); break;
	case THRIFT_MAP: snprintf(buf, n, ""); break;
	case THRIFT_STRUCT: snprintf(buf, n, "..."); break;
	default: snprintf(buf, n, "?"); break;
	}
}

void thrift_print_field(int32_t id, int32_t type, union thrift_value value)
{
	char buf[100] = {0};
	thrift_get_field_str(type, value, buf);
	printf("%i = %s : %s\n", id, buf, thrift_get_type_string(type));
}





void thrift_recursive_read(struct thrift_context * ctx, int32_t id, int32_t type)
{
    union thrift_value value = {0};
	switch (type)
    {
	case THRIFT_STOP:break;
	case THRIFT_STRUCT:
		ctx->last_field_id = 0;
		ctx->push(ctx, 0, type, value);
		while(1)
		{
			uint8_t modifier;
			int32_t id;
			modifier = (ctx->current[0] & 0xF0) >> 4;
			type = ctx->current[0] & 0x0F;
			ctx->current++;
			if(type == THRIFT_STOP){break;}
			if (modifier == 0)
			{
				id = thrift_read_varint_i64(ctx);
			}
			else
			{
				id = ctx->last_field_id + modifier;
			}
			ctx->last_field_id = id;
			thrift_recursive_read(ctx, id, type);
		}
		break;
	case THRIFT_BINARY:
		value.string_size = thrift_read_varint_i64(ctx);
		value.string_data = ecs_os_malloc(value.string_size);
		memcpy(value.string_data, ctx->current, value.string_size);
		ctx->current += value.string_size;
		ctx->push(ctx, id, type, value);
		break;
	case THRIFT_BOOLEAN_TRUE:
		value.value_u64 = 1;
		ctx->push(ctx, id, type, value);
		break;
	case THRIFT_BOOLEAN_FALSE:
		value.value_u64 = 1;
		ctx->push(ctx, id, type, value);
		break;
	case THRIFT_I32:
		value.value_i64 = thrift_read_zigzag_i64(ctx);
		ctx->push(ctx, id, type, value);
		break;
	case THRIFT_I64:
		value.value_i64 = thrift_read_zigzag_i64(ctx);
		ctx->push(ctx, id, type, value);
		break;
	case THRIFT_LIST:
		value.list_type = ctx->current[0] & 0x0F;
		value.list_size = (ctx->current[0] >> 4) & 0x0F;
		ctx->current++;
		if(value.list_size == 0xF)
		{
			value.list_size = thrift_read_varint_i64(ctx);
		}
		ctx->push(ctx, id, type, value);
		for(int i = 0; i < value.list_size; ++i)
		{
			thrift_recursive_read(ctx, id, value.list_type);
		}
		break;
	default:
		printf("Warning no type found %i!\n", type);
		break;
	}
}



