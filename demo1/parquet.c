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


void parquet_assigner_filemetadata(parquet_filemetadata_t *data, int32_t id, int32_t type, union thrift_value value)
{
    switch (id)
    {
    case 1:data->version = value.value_i64; break;
    case 2:data->schema = ecs_vector_new(parquet_schema_element_t, 0); break;
    case 3:data->num_rows = value.value_i64; break;
    default:break;
    }
};



void print_field1(int32_t id, int32_t type, union thrift_value value, int indent)
{
	char buf[100] = {0};
	thrift_get_field_str(type, value, buf, 100);
	for(int i = 0; i < indent; ++i){printf("    ");}
	switch (type)
	{
	case THRIFT_STRUCT:
		printf("%02i =\n", id);
		for(int i = 0; i < indent; ++i){printf("    ");}
		printf("{\n");
		break;
	case THRIFT_STOP:printf("}\n");break;
	}
	switch (type)
	{
	case THRIFT_STRUCT:break;
	case THRIFT_STOP:break;
	default:
		printf("%02i = %-20s : %s\n", id, buf, thrift_get_type_string(type));
		break;
	}
}


void parquet_footer_cb(struct thrift_context *ctx, int32_t id, int32_t type, union thrift_value value)
{
	print_field1(id, type, value, ctx->sp);
    parquet_reader_t *reader = (parquet_reader_t*)ctx;
    if(ctx->sp == 1 && ctx->stack_id[0] == 0)
    {
        parquet_assigner_filemetadata(&reader->meta, id, type, value);
    }
}













void parquet_read(parquet_reader_t * reader, char const * filename)
{
	FILE * file = NULL;
	ecs_os_fopen(&file, filename, "rb");
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
    reader->footer.data_start = ecs_os_malloc(l);
    reader->footer.data_current = reader->footer.data_start;
    reader->footer.data_end = reader->footer.data_start + l;
    reader->footer.cb_field = parquet_footer_cb;
    fread(reader->footer.data_start, l, 1, file); 

    thrift_recursive_read(&reader->footer, 0, THRIFT_STRUCT);

    // column first_name
    /*
    {
        int data_size = 1000000;
        int n = 1000;
        fseek(file, 17317, SEEK_SET);
        struct ByteArray * a = ecs_os_malloc_n(struct ByteArray, n);
        const uint8_t* data = ecs_os_malloc_n(uint8_t, data_size);
        fread(data, n, 1, file);
        parquet_read_bytearray(data, data_size, n, 1, a);
        printf("parquet_read_bytearray\n");
        for(int i = 0; i < n; ++i)
        {
		    printf("%4i %4i %08i: %.*s\n", i, a[i].len, (intptr_t)a[i].ptr - (intptr_t)data, a[i].len, a[i].ptr);
        }
        printf("parquet_read_bytearray\n");
    }
    */
	if(file) {fclose(file);}
	if(reader->footer.data_start) {ecs_os_free(reader->footer.data_start);}
}




