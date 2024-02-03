#include "data.h"

void* build_data(void *data, size_t data_sz, void *buffer, size_t buf_sz)
{
	uint8_t *data_buffer_base = buffer;

	memcpy(data_buffer_base, data, data_sz);

	return &data_buffer_base[data_sz];
}
