#include "ethernetII.h"

void *build_ethernetII(ethernetII *ethernet_layer, void *buffer, size_t buf_sz)
{
	size_t mac_sz = sizeof(ethernet_layer->destination_mac_address);
		
	uint8_t *buffer_tmp = (uint8_t*)buffer;

	memcpy(&buffer_tmp[0], &ethernet_layer->destination_mac_address, mac_sz);
	memcpy(&buffer_tmp[mac_sz], &ethernet_layer->source_mac_address, mac_sz);
	buffer_tmp[2 * mac_sz] = (uint8_t)((ethernet_layer->ethernet_type & 0xFF00) >> 8);	
	buffer_tmp[2 * mac_sz + 1] = (uint8_t)(ethernet_layer->ethernet_type & 0x00FF);
	
	return buffer_tmp + sizeof(ethernetII);
}
