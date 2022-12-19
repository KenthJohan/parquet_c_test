#include "parquet.h"
#include "thrift.h"
#include <stdlib.h>
#include <stdint.h>
#include "flecs.h"

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



struct ByteArray
{
    uint32_t len;
    char* ptr;
};

int parquet_read_bytearray(const uint8_t* data, int64_t data_size, int num_values, int type_length, struct ByteArray* out)
{
    int bytes_decoded = 0;
    int increment;
    for (int i = 0; i < num_values; ++i)
    {
        uint32_t len = (uint32_t)(*data);
        out[i].len = len;
        increment = sizeof(uint32_t) + len;
        if (data_size < increment) {return EOF;}
        out[i].ptr = data + sizeof(uint32_t);
        data += increment;
        data_size -= increment;
        bytes_decoded += increment;
    }
    return bytes_decoded;
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
    ctx->data_start = ecs_os_malloc(l);
    ctx->data_current = ctx->data_start;
    ctx->data_end = ctx->data_start + l;
    fread(ctx->data_start, l, 1, file);



    // column first_name
    {
        int n = 1000;
        fseek(file, 17317, SEEK_SET);
        struct ByteArray a[100] = {0};
        const uint8_t* data = ecs_os_malloc(n);
        fread(data, n, 1, file);
        parquet_read_bytearray(data, n, 100, 1, a);
        printf("parquet_read_bytearray\n");
    }

}




