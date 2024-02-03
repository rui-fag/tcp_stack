#include "udp.h"

void* build_udp(udp *transport_layer, void *buffer, size_t buf_sz)
{
	uint8_t *udp_buffer_base = buffer;
	size_t transport_layer_sz = sizeof(udp);

	udp_buffer_base[0] = (uint8_t)((transport_layer->source_port & 0xFF00) >> 8);
	udp_buffer_base[1] = (uint8_t)(transport_layer->source_port & 0x00FF);
	udp_buffer_base[2] = (uint8_t)((transport_layer->destination_port & 0xFF00) >> 8);
	udp_buffer_base[3] = (uint8_t)(transport_layer->destination_port & 0x00FF);
	udp_buffer_base[4] = (uint8_t)((transport_layer->length & 0xFF00) >> 8);
	udp_buffer_base[5] = (uint8_t)(transport_layer->length & 0x00FF);
	udp_buffer_base[6] = (uint8_t)((transport_layer->checksum & 0xFF00) >> 8);
	udp_buffer_base[7] = (uint8_t)(transport_layer->checksum & 0x00FF);

	return &udp_buffer_base[8]; 
}


uint16_t compute_checksum_ipv6(ipv6* ip_layer, udp* transport_layer, uint8_t* data, size_t data_sz)
{
	uint32_t ip_sum	= 0;

	for(size_t i = 0; i < 8; i++)
	{
		ip_sum += ip_layer->source_address[i];
		ip_sum += ip_layer->destination_address[i];
	}
	ip_sum += ip_layer->next_header;
	ip_sum += ip_layer->payload_length;

	uint32_t transport_sum = 0;

	transport_sum = transport_layer->source_port + transport_layer->destination_port + transport_layer->length;

	uint32_t data_sum = 0;

	for(size_t i = 0; i < data_sz; i += 2)
	{
		if(i + 1 >= data_sz)
		{
			uint16_t data_word = 0; 

			data_word = (uint16_t)(data[i] << 8);
			data_word |= 0;
			data_sum += data_word;
		}
		else
		{
			uint16_t data_word = 0; 

			data_word = (uint16_t)(data[i] << 8);
			data_word |= (uint16_t)(data[i + 1]);
			data_sum += data_word;
		}
	}

	uint32_t total_sum = data_sum + transport_sum + ip_sum;

	total_sum += ((total_sum & 0x00FF0000) >> 16);

	uint16_t checksum = (uint16_t)(total_sum ^ 0xFFFF);

	return checksum;
}
