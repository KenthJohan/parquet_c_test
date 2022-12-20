#include "thrift.h"
#include <stdint.h>
#include <ctype.h>
#include "flecs.h"

// https://github.com/apache/thrift/blob/master/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc

char const * thrift_get_type_string(uint32_t t)
{
	switch(t)
	{
	case THRIFT_STOP: return "STOP";
	case THRIFT_BOOLEAN_TRUE: return "BOOLEAN_TRUE";
	case THRIFT_BOOLEAN_FALSE: return "BOOLEAN_FALSE";
	case THRIFT_BYTE: return "BYTE";
	case THRIFT_I16: return "I16";
	case THRIFT_I32: return "I32";
	case THRIFT_I64: return "I64";
	case THRIFT_DOUBLE: return "DOUBLE";
	case THRIFT_BINARY: return "BINARY";
	case THRIFT_LIST: return "LIST";
	case THRIFT_SET: return "SET";
	case THRIFT_MAP: return "MAP";
	case THRIFT_STRUCT: return "STRUCT";
	default: return "";
	}
}


void string_friendly(char s[], int n)
{
	// Temorary start
	for(int i = 0; i < n; ++i)
	{
		if (s[i] == '\0') {break;}
		if ((s[i] >= 32) && (s[i] <= 126)) {continue;}
		s[i] = '?';
	}
	// Temorary end
}

void thrift_get_field_str(int32_t type, union thrift_value value, char * buf, int n)
{
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
	case THRIFT_BINARY:
		//string_friendly(value.string_data, value.string_size);
		snprintf(buf, n, "%.*s", value.string_size, value.string_data);
		break;
	case THRIFT_LIST: snprintf(buf, n, "%i of %s", value.list_size, thrift_get_type_string(value.list_type)); break;
	case THRIFT_SET: snprintf(buf, n, ""); break;
	case THRIFT_MAP: snprintf(buf, n, ""); break;
	case THRIFT_STRUCT: snprintf(buf, n, ""); break;
	default: snprintf(buf, n, "?"); break;
	}
}


uint8_t thrift_read_u8(struct thrift_context * ctx)
{
	uint8_t b = ctx->data_current[0];
	ctx->data_current++;
	return b;
}


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
		uint8_t b = thrift_read_u8(ctx);
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
	return (hi << 32) | (lo << 0);
}


int64_t thrift_read_zigzag_i64(struct thrift_context * ctx)
{
	int64_t value = thrift_read_varint_i64(ctx);
	return thrift_zigzag_to_i32(value);
}







void thrift_recursive_read(struct thrift_context * ctx, int32_t id, int32_t type)
{
	if(ctx->data_current >= ctx->data_end){goto no_more_data;}
    union thrift_value value = {0};
	uint8_t byte;
	uint8_t modifier;
	switch (type)
    {
	case THRIFT_STOP:
		break;
	case THRIFT_STRUCT:
		ctx->cb_field(ctx, id, type, value);
		ctx->stack_id[ctx->sp] = ctx->last_field_id;
		ctx->sp++;
		ctx->last_field_id = 0;
        while(1)
		{
			byte = thrift_read_u8(ctx);
			modifier = (byte & 0xF0) >> 4;
			type = byte & 0x0F;
			if(type == THRIFT_STOP)
			{
				ctx->sp--;
				ctx->last_field_id = ctx->stack_id[ctx->sp];
				ctx->cb_field(ctx, 0, type, value);
				break;
			}
			if(ctx->data_current >= ctx->data_end){goto no_more_data;}
			if (modifier == 0)
			{
				ctx->last_field_id = thrift_read_varint_i64(ctx);
			}
			else
			{
				ctx->last_field_id = ctx->last_field_id + modifier;
			}
			thrift_recursive_read(ctx, ctx->last_field_id, type);
		}
		break;
	case THRIFT_BINARY:
		value.string_size = thrift_read_varint_i64(ctx);
		value.string_data = NULL;
		if(value.string_size < 0){printf("error1!\n");goto error;}
		if(value.string_size > 0)
		{
			value.string_size = value.string_size < 100 ? value.string_size : 100; // temprary safeguard
			value.string_data = ecs_os_malloc(value.string_size+1);
			memcpy(value.string_data, ctx->data_current, value.string_size);
			value.string_data[value.string_size] = '\0';
			ctx->data_current += value.string_size;
		}
		ctx->cb_field(ctx, id, type, value);
		break;
	case THRIFT_BOOLEAN_TRUE:
		value.value_u64 = 1;
		ctx->cb_field(ctx, id, type, value);
		break;
	case THRIFT_BOOLEAN_FALSE:
		value.value_u64 = 1;
		ctx->cb_field(ctx, id, type, value);
		break;
	case THRIFT_I32:
		value.value_i64 = thrift_read_zigzag_i64(ctx);
		ctx->cb_field(ctx, id, type, value);
		break;
	case THRIFT_I64:
		value.value_i64 = thrift_read_zigzag_i64(ctx);
		ctx->cb_field(ctx, id, type, value);
		break;
	case THRIFT_LIST:
		byte = thrift_read_u8(ctx);
		value.list_type = byte & 0x0F;
		value.list_size = (byte >> 4) & 0x0F;
		if(ctx->data_current >= ctx->data_end){goto no_more_data;}
		if(value.list_size == 0xF)
		{
			value.list_size = thrift_read_varint_i64(ctx);
		}
		ctx->cb_field(ctx, id, type, value);
		ctx->sp++;
		for(int i = 0; i < value.list_size; ++i)
		{
			thrift_recursive_read(ctx, i, value.list_type);
		}
		ctx->sp--;
		break;
	default:
		printf("Warning no type found %i!\n", type);
		break;
	}
	return;
no_more_data:
	printf("No more data is available!\n");
	return;
error:
	return;
}

