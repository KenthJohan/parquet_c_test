#pragma once

#define ASSERTF(exp,format,...) assert(exp)
#define ASSERT(exp) assert(exp)

char * file_malloc(char const * filename, long * length);
