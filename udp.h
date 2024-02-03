#include <stdint.h>
#include <stddef.h>
#include "ipv6.h"

#ifndef __UDP_H__
#define __UDP_H__

typedef struct
{
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t length;
	uint16_t checksum;

}udp;

void *build_udp(udp*, void*, size_t);

uint16_t compute_checksum_ipv6(ipv6*, udp*, uint8_t*, size_t);

#endif
