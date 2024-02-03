#include "ipv6.h"

void *build_ipv6(ipv6 *ip_layer, void* buffer, size_t buf_sz)
{
	size_t ip_layer_sz = 4 * 10;
	uint8_t *ipv6_buffer_base = buffer;

	ipv6_buffer_base[0] = (uint8_t)ip_layer->version << 4;
	ipv6_buffer_base[0] |= 0x0F & (uint8_t)((ip_layer->traffic_class & 0xF0) >> 4);
	ipv6_buffer_base[1] = 0xF0 & (uint8_t)((ip_layer->traffic_class & 0x0F) << 4);
	ipv6_buffer_base[1] |= 0x0F & (uint8_t)((ip_layer->flow_label & 0xF0000) >> 16);
	ipv6_buffer_base[2] = (uint8_t)((ip_layer->flow_label & 0x0FF00) >> 8);
	ipv6_buffer_base[3] = (uint8_t)(ip_layer->flow_label & 0x000FF);
	ipv6_buffer_base[4] = (uint8_t)((ip_layer->payload_length & 0xFF00) >> 8);
	ipv6_buffer_base[5] = (uint8_t)(ip_layer->payload_length & 0x00FF);
	ipv6_buffer_base[6] = ip_layer->next_header;
	ipv6_buffer_base[7] = ip_layer->hop_limit;

	size_t index = 0;

	for(size_t i = 8; i < 8 + 4 * 4; i += 4)
	{
			ipv6_buffer_base[i] = (uint8_t)((ip_layer->source_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 1] = (uint8_t)(ip_layer->source_address[index] & 0x00FF);
			index++;
			ipv6_buffer_base[i + 2] = (uint8_t)((ip_layer->source_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 3] = (uint8_t)(ip_layer->source_address[index] & 0x00FF);
			index++;
	}
	
	index = 0;

	for(size_t i = 8 + 4 * 4; i < 8 + 4 * 4 + 4 * 4; i += 4)
	{
			ipv6_buffer_base[i] = (uint8_t)((ip_layer->destination_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 1] = (uint8_t)(ip_layer->destination_address[index] & 0x00FF);
			index++;
			ipv6_buffer_base[i + 2] = (uint8_t)((ip_layer->destination_address[index] & 0xFF00) >> 8);
			ipv6_buffer_base[i + 3] = (uint8_t)(ip_layer->destination_address[index] & 0x00FF);
			index++;
	}

	return &ipv6_buffer_base[IPV6_HDR_SZ];
}
