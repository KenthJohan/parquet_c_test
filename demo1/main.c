#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "flecs.h"
#include "parquet.h"






int main (int argc, char * argv [])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_IMPORT(world, FlecsUnits);
	parquet_reader_t reader = {0};
    parquet_read(&reader, "../demo1/userdata1.parquet");
	printf("Run ECS\n");
    return ecs_app_run(world, &(ecs_app_desc_t){
        .enable_rest = true
    });
}
