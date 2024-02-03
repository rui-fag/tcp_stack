#include "frame_check.h"

void *build_frame_check(frame_check *frame, void *buffer, size_t buf_sz)
{

	uint32_t *frame_check_ptr = buffer;
	size_t frame_check_sz = sizeof(frame->frame_check);
	
	frame_check_ptr[0] = htonl(frame->frame_check);

	return &frame_check_ptr[frame_check_sz/ sizeof(uint32_t)];
}
