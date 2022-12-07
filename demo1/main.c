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
	case THRIFT_STRUCT:printf("{\n");break;
	case THRIFT_STOP:printf("}\n");break;
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
	char buf[100] = {0};
	ecs_entity_t e = 0;
	switch (type)
	{
	case THRIFT_STRUCT:
		ctx->sp++;
		ctx->scope[ctx->sp] = ecs_new(ctx->world, 0);
		snprintf(buf, 100, "struct%i", id);
		ecs_doc_set_name(ctx->world, ctx->scope[ctx->sp], buf);
		ecs_set_scope(ctx->world, ctx->scope[ctx->sp]);
		break;
	case THRIFT_STOP:
		ctx->sp--;
		ecs_set_scope(ctx->world, ctx->scope[ctx->sp]);
		break;
	case THRIFT_BINARY:
		e = ecs_new(ctx->world, 0);
		snprintf(buf, 100, "string_%i_%s", id, value.string_data ? value.string_data : "null");
		ecs_doc_set_name(ctx->world, e, buf);
		break;
	case THRIFT_I16:
	case THRIFT_I32:
		e = ecs_new(ctx->world, 0);
		snprintf(buf, 100, "int_%i_%jd", id, value.value_i64);
		ecs_doc_set_name(ctx->world, e, buf);
		break;
	}
	
    print_field(id, type, value);
	//printf("%p %p\n", ctx->data_current, ctx->data_end);
}

void read_a_parquet_file(struct thrift_context ctx, char const * filename)
{
	FILE * file = NULL;
	ecs_os_fopen(&file, filename, "rb");
	int32_t l = 0;
    ctx.cb_field = cb_field;
    parquet_read_footer(&ctx, file);
	thrift_recursive_read(&ctx, 0, THRIFT_STRUCT);
	if(file) {fclose(file);}
	if(ctx.data_start) {ecs_os_free(ctx.data_start);}
}



int main (int argc, char * argv [])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);
	struct thrift_context ctx = {0};
	ctx.scope[0] = ecs_new_entity(world, "Sun");
	ecs_set_scope(world, ctx.scope[0]);
	ctx.world = world;
    read_a_parquet_file(ctx, "../demo1/userdata1.parquet");
	printf("Run ECS\n");
    return ecs_app_run(world, &(ecs_app_desc_t){
        .enable_rest = true
    });
}
