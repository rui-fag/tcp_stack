#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifndef __ETHERNETII_H__
#define __ETHERNETII_H__

typedef struct
{
	uint8_t destination_mac_address[6];
	uint8_t source_mac_address[6];
	uint16_t ethernet_type;
}ethernetII;

void *build_ethernetII(ethernetII *, void*, size_t);

#endif
