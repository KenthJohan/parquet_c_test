#include "thrift.h"
#include <stdint.h>
#include "flecs.h"

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
		uint8_t b = ctx->data_current[0];
		ctx->data_current++;
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
	case THRIFT_STRUCT: snprintf(buf, n, ""); break;
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
	if(ctx->data_current >= ctx->data_end){goto no_more_data;}
    union thrift_value value = {0};
	uint8_t byte;
	switch (type)
    {
	case THRIFT_STOP:break;
	case THRIFT_STRUCT:
		ctx->last_field_id = 0;
		ctx->cb_field(ctx, 0, type, value);
        while(1)
		{
			uint8_t modifier;
			int32_t id;
			byte = ctx->data_current[0];
			ctx->data_current++;
			modifier = (byte & 0xF0) >> 4;
			type = byte & 0x0F;
			if(type == THRIFT_STOP)
			{
				ctx->cb_field(ctx, 0, type, value);
				break;
			}
			if(ctx->data_current >= ctx->data_end){goto no_more_data;}
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
		value.string_data = NULL;
		if(value.string_size < 0){printf("error1!\n");goto error;}
		if(value.string_size > 10000){printf("error3!\n");goto error;}
		if(value.string_size > 0)
		{
			value.string_data = ecs_os_malloc(value.string_size);
			memcpy(value.string_data, ctx->data_current, value.string_size);
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
		byte = ctx->data_current[0];
		value.list_type = byte & 0x0F;
		value.list_size = (byte >> 4) & 0x0F;
		ctx->data_current++;
		if(ctx->data_current >= ctx->data_end){goto no_more_data;}
		if(value.list_size == 0xF)
		{
			value.list_size = thrift_read_varint_i64(ctx);
		}
		ctx->cb_field(ctx, id, type, value);
		for(int i = 0; i < value.list_size; ++i)
		{
			thrift_recursive_read(ctx, id, value.list_type);
		}
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



