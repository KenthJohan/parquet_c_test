#pragma once
#include "misc.h"
#include <flecs.h>
#include <stdio.h>



char * file_malloc(char const * filename, long * length)
{
	FILE * file;
	ecs_os_fopen(&file, filename, "rb");
	if (file == NULL) {goto error;}

	int r;
	r = fseek(file, 0, SEEK_END);
	if (r != 0) {goto error;}

	*length = ftell(file);
	if((*length) <= 0) {goto error;}

	r = fseek(file, 0, SEEK_SET);
	if (r != 0) {goto error;}

	char * buffer = (char *) ecs_os_malloc ((unsigned) (*length) + 1);
	if(buffer == NULL) {goto error;}

	memset(buffer, 0, (unsigned) (*length) + 1);
	//buffer [length + 1] = 0;
	if (*length > 0)
	{
		size_t n = fread(buffer, (unsigned) (*length), 1, file);
		ASSERTF (n == 1, "fread error %i %i", (int)n, (int)(*length));
		if(n != 1) {goto error;}
	}
	fclose(file);
	return buffer;
error:
	if(file) {fclose(file);}
	if(buffer) {ecs_os_free(buffer);}
	return NULL;
}
