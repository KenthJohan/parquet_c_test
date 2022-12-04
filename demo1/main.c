#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <flecs.h>
#include "thrift.h"
#include "par.h"
#include "misc.h"









int main (int argc, char * argv [])
{
	ecs_world_t *world = ecs_init();
	par_read();

	//long l;
	//uint8_t * f = file_malloc("../demo1/userdata1.parquet", &l);
	//test(f);
	//meta.version = swap_int32(meta.version);
	//meta.length = swap_int32(meta.length);
	printf("EXIT_SUCCESS\n");
	ecs_fini(world);
	return EXIT_SUCCESS;
}
