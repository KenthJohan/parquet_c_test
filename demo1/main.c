#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"
#include "parquet.h"


void print_field(int32_t id, int32_t type, union thrift_value value)
{
	static int indent = -1;
	char buf[100] = {0};
	thrift_get_field_str(type, value, buf);
	for(int i = 0; i < indent; ++i){printf(" ");}
	switch (type)
	{
	case THRIFT_STRUCT:
		printf("{\n");
		break;
	case THRIFT_STOP:
		printf("}\n");
		break;
	}
	indent += (type == THRIFT_STRUCT);
	indent -= (type == THRIFT_STOP);
	switch (type)
	{
	case THRIFT_STRUCT:break;
	case THRIFT_STOP:break;
	default:
		printf("%02i = %-20s : %s\n", id, buf, thrift_get_type_string(type));
		break;
	}
}
	


void cb_field(struct thrift_context * ctx, int32_t id, int32_t type, union thrift_value value)
{
    print_field(id, type, value);
	//printf("%p %p\n", ctx->data_current, ctx->data_end);
}

void read_a_parquet_file(char const * filename)
{
	FILE * file = NULL;
	ecs_os_fopen(&file, filename, "rb");
	int32_t l = 0;
	struct thrift_context ctx = {0};
    ctx.cb_field = cb_field;
    parquet_read_footer(&ctx, file);
	thrift_recursive_read(&ctx, 0, THRIFT_STRUCT);
	if(file) {fclose(file);}
	if(ctx.data_start) {ecs_os_free(ctx.data_start);}
}


int main (int argc, char * argv [])
{
	ecs_world_t *world = ecs_init();
    read_a_parquet_file("../demo1/userdata1.parquet");
	printf("EXIT_SUCCESS\n");
	ecs_fini(world);
	return EXIT_SUCCESS;
}
