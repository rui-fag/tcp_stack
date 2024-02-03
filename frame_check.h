#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#ifndef __FRAME_CHECK__
#define __FRAME_CHECK__

typedef struct
{
	uint32_t frame_check;	
}frame_check;

void *build_frame_check(frame_check*, void*, size_t);

#endif
